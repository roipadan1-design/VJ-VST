import * as THREE from 'three'
import { GLTFLoader } from 'three/addons/loaders/GLTFLoader.js'
import { MeshSurfaceSampler } from 'three/addons/math/MeshSurfaceSampler.js'

const SURF_PTS   = 95000
const INNER_FRAC = 0.20
const ACCENT_PCT = 0.009

function textureToSampler(texture) {
  const img = texture.image
  if (!img) return null
  const w = img.width || img.naturalWidth || 1024
  const h = img.height || img.naturalHeight || 1024
  const cv = document.createElement('canvas')
  cv.width = w
  cv.height = h
  const ctx = cv.getContext('2d', { willReadFrequently: true })
  ctx.drawImage(img, 0, 0, w, h)
  const data = ctx.getImageData(0, 0, w, h).data
  return { data, w, h }
}

function sampleBrightness(tex, u, v) {
  if (!tex) return 0.7
  const x = Math.min(tex.w - 1, Math.max(0, Math.floor(u * tex.w)))
  const y = Math.min(tex.h - 1, Math.max(0, Math.floor((1 - v) * tex.h)))
  const idx = (y * tex.w + x) * 4
  return tex.data[idx] / 255 * 0.299 + tex.data[idx + 1] / 255 * 0.587 + tex.data[idx + 2] / 255 * 0.114
}

export async function buildHeadGeometry(onProgress) {
  const [gltf, colorMap] = await Promise.all([
    new Promise((resolve, reject) => {
      new GLTFLoader().load('/models/LeePerrySmith.glb', resolve,
        (e) => onProgress && onProgress(e.loaded / e.total), reject)
    }),
    new Promise((resolve, reject) => {
      new THREE.TextureLoader().load('/models/LeePerrySmith-COL.jpg', resolve, undefined, reject)
    }),
  ])

  let mesh = null
  gltf.scene.traverse(o => { if (o.isMesh && !mesh) mesh = o })
  if (!mesh) throw new Error('No mesh found in GLB')
  mesh.updateWorldMatrix(true, true)

  // LeePerrySmith.glb ships no embedded color map — use the downloaded diffuse.
  const map = colorMap
  const tex = textureToSampler(map)

  const sampler = new MeshSurfaceSampler(mesh).build()

  const totalPts   = Math.floor(SURF_PTS * (1 + INNER_FRAC))
  const innerStart = SURF_PTS
  const positions  = new Float32Array(totalPts * 3)
  const normals    = new Float32Array(totalPts * 3)
  const rands      = new Float32Array(totalPts)
  const accents    = new Float32Array(totalPts)
  const inners     = new Float32Array(totalPts)
  const colors     = new Float32Array(totalPts)

  const _pos = new THREE.Vector3()
  const _nor = new THREE.Vector3()
  const _uv  = new THREE.Vector2()

  for (let i = 0; i < SURF_PTS; i++) {
    sampler.sample(_pos, _nor, undefined, _uv)
    positions[i*3] = _pos.x; positions[i*3+1] = _pos.y; positions[i*3+2] = _pos.z
    normals[i*3] = _nor.x; normals[i*3+1] = _nor.y; normals[i*3+2] = _nor.z
    rands[i]   = Math.random()
    accents[i] = Math.random() < ACCENT_PCT ? 1 : 0
    inners[i]  = 0
    colors[i]  = sampleBrightness(tex, _uv.x, _uv.y)
  }
  for (let i = innerStart; i < totalPts; i++) {
    sampler.sample(_pos, _nor, undefined, _uv)
    const d = 0.015 + Math.random() * 0.10
    positions[i*3] = _pos.x - _nor.x*d; positions[i*3+1] = _pos.y - _nor.y*d; positions[i*3+2] = _pos.z - _nor.z*d
    normals[i*3] = -_nor.x; normals[i*3+1] = -_nor.y; normals[i*3+2] = -_nor.z
    rands[i]   = Math.random()
    accents[i] = 0
    inners[i]  = 1
    colors[i]  = sampleBrightness(tex, _uv.x, _uv.y) * 0.6
  }

  // Normalize from the MESH bounding box so points + surface mesh share one frame
  const meshGeo = mesh.geometry.clone()
  meshGeo.applyMatrix4(mesh.matrixWorld)
  meshGeo.computeBoundingBox()
  const bb = meshGeo.boundingBox
  const cx = (bb.min.x + bb.max.x) / 2
  const cy = (bb.min.y + bb.max.y) / 2
  const cz = (bb.min.z + bb.max.z) / 2
  const scale = 2.3 / (bb.max.y - bb.min.y)

  for (let i = 0; i < totalPts; i++) {
    positions[i*3]   = (positions[i*3]   - cx) * scale
    positions[i*3+1] = (positions[i*3+1] - cy) * scale
    positions[i*3+2] = (positions[i*3+2] - cz) * scale
  }

  // Bake the same transform into the surface mesh: p' = scale * (p - center)
  const norm = new THREE.Matrix4()
    .makeScale(scale, scale, scale)
    .multiply(new THREE.Matrix4().makeTranslation(-cx, -cy, -cz))
  meshGeo.applyMatrix4(norm)
  meshGeo.computeVertexNormals()

  const geo = new THREE.BufferGeometry()
  geo.setAttribute('position', new THREE.BufferAttribute(positions, 3))
  geo.setAttribute('aNormal',  new THREE.BufferAttribute(normals,   3))
  geo.setAttribute('aRand',    new THREE.BufferAttribute(rands,     1))
  geo.setAttribute('aAccent',  new THREE.BufferAttribute(accents,   1))
  geo.setAttribute('aInner',   new THREE.BufferAttribute(inners,    1))
  geo.setAttribute('aColor',   new THREE.BufferAttribute(colors,    1))
  geo.computeBoundingBox()

  const anchors = [
    new THREE.Vector3( 0.05,  0.92, 0.30),
    new THREE.Vector3(-0.50,  0.35, 0.25),
    new THREE.Vector3( 0.52,  0.28, 0.22),
    new THREE.Vector3( 0.00, -0.10, 0.55),
    new THREE.Vector3(-0.30, -0.55, 0.35),
    new THREE.Vector3( 0.35, -0.62, 0.30),
  ]

  return { geo, anchors, meshGeo, map }
}

import * as THREE from 'three'

export function createCage(geo) {
  geo.computeBoundingBox()
  const bb  = geo.boundingBox
  const sz  = new THREE.Vector3()
  bb.getSize(sz)
  const ctr = new THREE.Vector3()
  bb.getCenter(ctr)

  // Expand slightly
  sz.multiplyScalar(1.06)

  const box = new THREE.BoxGeometry(sz.x, sz.y, sz.z)
  const edges = new THREE.EdgesGeometry(box)
  const mat   = new THREE.LineBasicMaterial({
    color:       0xffffff,
    transparent: true,
    opacity:     0.28,
    depthTest:   false,
    blending:    THREE.AdditiveBlending,
  })
  const cage = new THREE.LineSegments(edges, mat)
  cage.position.copy(ctr)

  // Trajectory lines between random vertex pairs
  const positions = geo.attributes.position.array
  const n         = geo.attributes.position.count
  const LINE_CNT  = 18
  const lineVerts = new Float32Array(LINE_CNT * 2 * 3)

  for (let i = 0; i < LINE_CNT; i++) {
    const a = Math.floor(Math.random() * n)
    const b = Math.floor(Math.random() * n)
    lineVerts[i * 6]     = positions[a * 3]
    lineVerts[i * 6 + 1] = positions[a * 3 + 1]
    lineVerts[i * 6 + 2] = positions[a * 3 + 2]
    lineVerts[i * 6 + 3] = positions[b * 3]
    lineVerts[i * 6 + 4] = positions[b * 3 + 1]
    lineVerts[i * 6 + 5] = positions[b * 3 + 2]
  }

  const trajGeo = new THREE.BufferGeometry()
  trajGeo.setAttribute('position', new THREE.BufferAttribute(lineVerts, 3))
  const trajMat  = new THREE.LineBasicMaterial({
    color:       0x4488ff,
    transparent: true,
    opacity:     0.07,
    depthTest:   false,
    blending:    THREE.AdditiveBlending,
  })
  const traj = new THREE.LineSegments(trajGeo, trajMat)

  const group = new THREE.Group()
  group.add(cage)
  group.add(traj)
  return group
}

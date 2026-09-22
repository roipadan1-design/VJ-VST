import { chromium } from 'playwright'
import { mkdir, writeFile } from 'fs/promises'
import { resolve } from 'path'

const OUT = resolve('./reference')
await mkdir(OUT, { recursive: true })

// Vite dev server must be running on :5173 — video served at /ref.mp4 (same origin, no taint)
const browser = await chromium.launch({
  channel: 'chrome',
  headless: false,
  args: ['--autoplay-policy=no-user-gesture-required'],
})
const page = await browser.newPage({ viewport: { width: 1080, height: 1920 } })

// Land on the dev-server origin first so /ref.mp4 is same-origin (no canvas taint)
await page.goto('http://localhost:5173/', { waitUntil: 'domcontentloaded' }).catch(() => {})

await page.setContent(`
<body style="margin:0;background:#000">
  <video id="v" src="/ref.mp4" muted crossorigin="anonymous" playsinline></video>
  <canvas id="c"></canvas>
</body>
`, { waitUntil: 'domcontentloaded' })

const meta = await page.evaluate(async () => {
  const v = document.getElementById('v')
  v.load()
  await new Promise((res, rej) => {
    if (v.readyState >= 1) return res()
    v.onloadedmetadata = res
    v.onerror = () => rej(new Error('err code ' + (v.error && v.error.code)))
    setTimeout(() => rej(new Error('metadata timeout, readyState=' + v.readyState)), 15000)
  }).catch(e => { throw e })
  return { duration: v.duration, w: v.videoWidth, h: v.videoHeight, rs: v.readyState }
})
console.log('Video:', JSON.stringify(meta))

const N = 12
const times = []
for (let i = 0; i < N; i++) times.push((meta.duration * (i + 0.5)) / N)

for (let i = 0; i < times.length; i++) {
  const t = times[i]
  const dataUrl = await page.evaluate(async (time) => {
    const v = document.getElementById('v')
    const c = document.getElementById('c')
    await new Promise((res) => {
      v.onseeked = res
      v.currentTime = time
      setTimeout(res, 4000)
    })
    c.width = v.videoWidth
    c.height = v.videoHeight
    c.getContext('2d').drawImage(v, 0, 0)
    return c.toDataURL('image/png')
  }, t)

  const b64 = dataUrl.replace(/^data:image\/png;base64,/, '')
  const name = `ref_${String(i).padStart(2, '0')}_t${t.toFixed(1)}.png`
  await writeFile(resolve(OUT, name), Buffer.from(b64, 'base64'))
  console.log(`  saved ${name}`)
}

await browser.close()
console.log('Done.')

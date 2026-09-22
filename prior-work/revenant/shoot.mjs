// Playwright screenshot harness — run AFTER `npm run dev` is up
// Usage: node shoot.mjs [phase0 phase1 ...]
import { chromium } from 'playwright'
import { mkdir }    from 'fs/promises'
import { resolve }  from 'path'

const PHASES   = process.argv.slice(2).map(Number).filter(n => !isNaN(n))
const DEFAULTS = [0.0, 0.13, 0.30, 0.46, 0.61, 0.80, 0.92]
const phases   = PHASES.length ? PHASES : DEFAULTS

const SHOTS_DIR = resolve('./shots')
await mkdir(SHOTS_DIR, { recursive: true })

const browser = await chromium.launch()
const page    = await browser.newPage({
  viewport:        { width: 540, height: 960 },
  deviceScaleFactor: 2,
})

console.log('Navigating to http://localhost:5173 …')
await page.goto('http://localhost:5173')

// Wait until model loads (window.__loaded becomes true)
await page.waitForFunction('window.__loaded === true', { timeout: 30000 })
  .catch(() => { console.warn('Model did not signal ready — proceeding anyway') })

// Extra settle time for first frame
await page.waitForTimeout(1500)

for (const p of phases) {
  await page.evaluate(ph => window.__setPhase(ph), p)
  await page.waitForTimeout(300)

  const name = `shot_${String(p).replace('.', '_')}.png`
  const path = resolve(SHOTS_DIR, name)
  await page.screenshot({ path })
  console.log(`  saved → shots/${name}`)
}

// Unfreeze
await page.evaluate(() => window.__unfreeze())

await browser.close()
console.log('Done.')

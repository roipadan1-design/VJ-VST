// Deterministic frame-by-frame capture: drives the page's window.__setFrame(i)
// for every frame instead of relying on real-time playback + screen
// recording, so output quality/timing is decoupled from how fast the GPU
// can actually run the shader (same idea as an offline renderer).
import puppeteer from 'puppeteer';
import fs from 'node:fs';
import path from 'node:path';

const PORT = process.env.PORT || 5173;
const WIDTH = Number(process.env.W) || 1080;
const HEIGHT = Number(process.env.H) || 1920;
const FPS = Number(process.env.FPS) || 30;
const DUR = Number(process.env.DUR) || 6;
const SEED = Number(process.env.SEED) || 7;

const outDir = path.join(process.cwd(), 'frames');
fs.rmSync(outDir, { recursive: true, force: true });
fs.mkdirSync(outDir, { recursive: true });

const url = `http://localhost:${PORT}/index.html?capture=1&w=${WIDTH}&h=${HEIGHT}&fps=${FPS}&dur=${DUR}&seed=${SEED}`;

const browser = await puppeteer.launch({
  headless: true,
  args: ['--enable-webgl', '--ignore-gpu-blocklist', '--enable-gpu-rasterization'],
  defaultViewport: { width: WIDTH, height: HEIGHT, deviceScaleFactor: 1 },
});

try {
  const page = await browser.newPage();
  page.on('console', (msg) => console.log('[page]', msg.text()));
  page.on('pageerror', (err) => console.error('[page error]', err));

  console.log('navigating to', url);
  await page.goto(url, { waitUntil: 'load' });

  await page.waitForFunction('window.__ready === true', { timeout: 60000 });
  const cfg = await page.evaluate('window.__CONFIG');
  console.log('config from page:', cfg);

  const total = cfg.totalFrames;
  const t0 = Date.now();
  for (let i = 0; i < total; i++) {
    await page.evaluate((idx) => window.__setFrame(idx), i);
    const dataUrl = await page.evaluate(() => document.querySelector('canvas').toDataURL('image/png'));
    const base64 = dataUrl.slice(dataUrl.indexOf(',') + 1);
    const name = `frame_${String(i).padStart(5, '0')}.png`;
    fs.writeFileSync(path.join(outDir, name), Buffer.from(base64, 'base64'));
    if (i % 10 === 0 || i === total - 1) {
      const elapsed = ((Date.now() - t0) / 1000).toFixed(1);
      console.log(`frame ${i + 1}/${total}  (${elapsed}s elapsed)`);
    }
  }

  console.log('done. frames written to', outDir);
} finally {
  await browser.close();
}

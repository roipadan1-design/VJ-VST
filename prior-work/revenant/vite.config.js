import { defineConfig } from 'vite'

// Build a SINGLE classic (IIFE) bundle instead of ES modules, and strip
// type="module"/crossorigin from index.html. Max's jweb runs classic <script>
// reliably (that's how the spectro-tunnel engine loaded); ES modules can fail.
function classicScript() {
  return {
    name: 'classic-script',
    enforce: 'post',
    transformIndexHtml(html) {
      // `defer` so the classic script runs after the DOM is parsed (modules
      // defer by default; a bare classic script in <head> would run too early).
      return html
        .replace(/<script type="module"\s+crossorigin\s+src="([^"]+)"><\/script>/g, '<script defer src="$1"></script>')
        .replace(/<link rel="stylesheet"\s+crossorigin\s+href=/g, '<link rel="stylesheet" href=')
        .replace(/\s+crossorigin/g, '')
    },
  }
}

export default defineConfig({
  base: './',
  build: {
    target: 'es2019',
    cssCodeSplit: false,
    modulePreload: false,
    assetsInlineLimit: 0,
    rollupOptions: {
      output: {
        format: 'iife',
        inlineDynamicImports: true,
        entryFileNames: 'assets/bundle.js',
        assetFileNames: 'assets/[name][extname]',
      },
    },
  },
  plugins: [classicScript()],
})

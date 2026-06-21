// build.mjs
import { build } from 'esbuild'
import fs from 'fs/promises'

await fs.mkdir('dist', { recursive: true })
await fs.copyFile('build/libe57-js.js', 'dist/libe57-js.js')
await fs.copyFile('build/libe57-js.wasm', 'dist/libe57-js.wasm')

await build({
  entryPoints: ['src/js/index.js'],
  bundle: true,
  platform: 'node',
  format: 'esm',
  packages: 'external',
  minify: true,
  outfile: 'dist/index.mjs',
})

await build({
  entryPoints: ['src/js/index.js'],
  bundle: true,
  platform: 'browser',
  format: 'esm',
  minify: true,
  outfile: 'dist/index.browser.mjs',
  plugins: [{
    name: 'browser-stubs',
    setup(build) {
      const stubs = {
        'sharp': 'export default () => { throw new Error("sharp is not available in the browser") }',
        'fs':    'export default () => { throw new Error("fs is not available in the browser") }',
        'path':  'export default () => { throw new Error("path is not available in the browser") }',
      }
      // Rewrite the relative libe57-js.js import to the dist-relative path and mark external
      build.onResolve({ filter: /libe57-js\.js$/ }, () => ({ path: './libe57-js.js', external: true }))
      build.onResolve({ filter: /^(sharp|fs|path)$/ }, args => ({ path: args.path, namespace: 'browser-stubs' }))
      build.onLoad({ filter: /.*/, namespace: 'browser-stubs' }, args => ({
        contents: stubs[args.path],
        loader: 'js',
      }))
    },
  }],
})

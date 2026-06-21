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
        'sharp': 'export default () => ({ metadata: async () => ({ width: 0, height: 0 }) })',
        'fs':    'export default {}; export const promises = {}; export const readFileSync = () => { throw new Error("fs not available in browser") }; export const writeFile = () => { throw new Error("fs not available in browser") }; export const existsSync = () => false; export const statSync = () => { throw new Error("fs not available in browser") }',
        'path':  'const join = (...p) => p.join("/").replace(/\\/+/g, "/"); const resolve = p => p; const extname = p => { const i = p.lastIndexOf("."); return i > p.lastIndexOf("/") ? p.slice(i) : "" }; const dirname = p => p.slice(0, p.lastIndexOf("/")) || "."; const basename = (p, e) => { const b = p.split("/").pop(); return e && b.endsWith(e) ? b.slice(0, -e.length) : b }; export default { join, resolve, extname, dirname, basename }; export { join, resolve, extname, dirname, basename }',
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

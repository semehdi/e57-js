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

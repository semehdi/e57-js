import path from 'path'
import { fileURLToPath } from 'url'
import fs from 'fs/promises'

const filename = fileURLToPath(import.meta.url)
const currentDir = path.dirname(filename)

const cJsFilePath = path.join(currentDir, "../build/libe57-js.js");
const wasmFilePath = path.join(currentDir, "../build/libe57-js.wasm")
const cJsFileDestPath = path.join(currentDir, "../dist/libe57-js.js");
const wasmFileDestPath = path.join(currentDir, "../dist/libe57-js.wasm")

await fs.copyFile(cJsFilePath, cJsFileDestPath)
await fs.copyFile(wasmFilePath, wasmFileDestPath)

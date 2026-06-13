import fs from 'fs'
import { E57, E57Reader } from './dist/index.mjs'

const filePath = process.argv[2]
if (!filePath) {
    console.error('Usage: node test_read.mjs <path-to-file.e57>')
    process.exit(1)
}

await E57.Init()

const buffer = fs.readFileSync(filePath)
const reader = E57Reader.FromBuffer(buffer)

console.log('Scans:', reader.GetData3DCount())
console.log('Images:', reader.GetImage2DCount())

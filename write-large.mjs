import { E57, E57Writer } from './dist/index.mjs'

const SCAN_COUNT  = 300
const PTS_PER_SCAN = 3_000_000
const OUT_FILE    = 'large.e57'

await E57.Init()

const writer = new E57Writer(OUT_FILE)

for (let s = 0; s < SCAN_COUNT; s++) {
    const header = new E57.LibE57.Data3D()
    header.guid = `scan-${s}`
    header.pointFields.cartesianXField = true
    header.pointFields.cartesianYField = true
    header.pointFields.cartesianZField = true

    const points = new Array(PTS_PER_SCAN)
    for (let i = 0; i < PTS_PER_SCAN; i++) {
        const pt = new E57.LibE57.Point()
        pt.cartesianX = i * 0.001
        pt.cartesianY = s * 1.0
        pt.cartesianZ = 0.0
        points[i] = pt
    }

    writer.AddScanSync(header, points)
    for (const pt of points) pt.delete()
    process.stdout.write(`\r  scan ${s + 1}/${SCAN_COUNT}`)
}

process.stdout.write('\n')
writer.Close()
console.log(`Done → ${OUT_FILE}`)

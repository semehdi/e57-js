import { describe, it, before, after } from 'node:test'
import assert from 'node:assert/strict'
import fs from 'node:fs'
import path from 'node:path'
import { fileURLToPath } from 'node:url'
import { E57, E57Writer, E57WriterImage } from '../dist/index.mjs'
import * as testsUtils from './utils.js'

const __dirname = path.dirname(fileURLToPath(import.meta.url))
const OUTPUT_DIR = path.join(__dirname, 'output')
const IMAGE_PATH = path.join(__dirname, 'data/images/image_1.jpg')

describe('SimpleWriter async', () => {
    before(async () => {
        await E57.Init()
        fs.mkdirSync(OUTPUT_DIR, { recursive: true })
    })

    after(() => {
        fs.rmSync(OUTPUT_DIR, { recursive: true, force: true })
    })

    it('AddScan resolves with scan index', async () => {
        const filePath = path.join(OUTPUT_DIR, 'AsyncScan.e57')
        const writer = new E57Writer(filePath)
        const idx = await testsUtils.withKeepAlive(writer.AddScan(testsUtils.makeCartesianHeader('Async Scan GUID'), testsUtils.makePoints(64)))
        assert.equal(Number(idx), 0)
        writer.Close()
        assert.ok(fs.existsSync(filePath))
    })

    it('AddScan sequential scans return incrementing indices', async () => {
        const filePath = path.join(OUTPUT_DIR, 'AsyncMultipleScans.e57')
        const writer = new E57Writer(filePath)
        const header = testsUtils.makeCartesianHeader('Async Scan GUID')

        header.guid = 'Async Scan 1 GUID'
        const idx0 = await testsUtils.withKeepAlive(writer.AddScan(header, testsUtils.makePoints(8)))

        header.guid = 'Async Scan 2 GUID'
        const idx1 = await testsUtils.withKeepAlive(writer.AddScan(header, testsUtils.makePoints(8)))

        header.guid = 'Async Scan 3 GUID'
        const idx2 = await testsUtils.withKeepAlive(writer.AddScan(header, testsUtils.makePoints(8)))

        assert.equal(Number(idx0), 0)
        assert.equal(Number(idx1), 1)
        assert.equal(Number(idx2), 2)

        writer.Close()
        assert.ok(fs.existsSync(filePath))
    })

    it('AddScan with 1025 points', async () => {
        const filePath = path.join(OUTPUT_DIR, 'AsyncCartesianPoints.e57')
        const writer = new E57Writer(filePath)
        const idx = await testsUtils.withKeepAlive(writer.AddScan(testsUtils.makeCartesianHeader('Async 1025 Points GUID'), testsUtils.makePoints(1025)))
        assert.equal(Number(idx), 0)
        writer.Close()
        assert.ok(fs.existsSync(filePath))
    })

    it('AddScan with zero points', async () => {
        const filePath = path.join(OUTPUT_DIR, 'AsyncZeroPoints.e57')
        const writer = new E57Writer(filePath)
        const header = testsUtils.makeCartesianHeader('Async Zero Points GUID')
        header.cartesianBounds.xMinimum = 0.0
        const idx = await testsUtils.withKeepAlive(writer.AddScan(header, []))
        assert.equal(Number(idx), 0)
        writer.Close()
        assert.ok(fs.existsSync(filePath))
    })

    it('AddImage multiple images', async () => {
        const filePath = path.join(OUTPUT_DIR, 'AsyncMultipleImages.e57')
        const writer = new E57Writer(filePath)

        const makeImage = (name) => {
            const img = new E57WriterImage(
                IMAGE_PATH,
                E57.LibE57.Image2DType.ImageJPEG,
                E57.LibE57.Image2DProjection.ProjectionVisual
            )
            img.setName(name)
            return img
        }

        await testsUtils.withKeepAlive(writer.AddImage(makeImage('Camera 1')))
        await testsUtils.withKeepAlive(writer.AddImage(makeImage('Camera 2')))
        await testsUtils.withKeepAlive(writer.AddImage(makeImage('Camera 3')))
        writer.Close()

        const reader = testsUtils.openReader(filePath)
        assert.equal(Number(reader.GetImage2DCount()), 3)
        assert.equal(reader.GetImage(0).GetHeader().name, 'Camera 1')
        assert.equal(reader.GetImage(1).GetHeader().name, 'Camera 2')
        assert.equal(reader.GetImage(2).GetHeader().name, 'Camera 3')
    })

    it('AddImage bytes match source file size', async () => {
        const filePath = path.join(OUTPUT_DIR, 'AsyncImageBytes.e57')
        const writer = new E57Writer(filePath)
        const image = new E57WriterImage(
            IMAGE_PATH,
            E57.LibE57.Image2DType.ImageJPEG,
            E57.LibE57.Image2DProjection.ProjectionVisual
        )

        const bytes = await testsUtils.withKeepAlive(writer.AddImage(image))
        writer.Close()

        const sourceSize = fs.statSync(IMAGE_PATH).size
        assert.equal(Number(bytes), sourceSize)
    })

    it('AddImage with rotation', async () => {
        const filePath = path.join(OUTPUT_DIR, 'AsyncImageRotation.e57')
        const writer = new E57Writer(filePath)
        const image = new E57WriterImage(
            IMAGE_PATH,
            E57.LibE57.Image2DType.ImageJPEG,
            E57.LibE57.Image2DProjection.ProjectionVisual
        )
        image.setName('Rotated camera')
        image.setRotation(1.0, 0.0, 0.0, 0.0)
        await testsUtils.withKeepAlive(writer.AddImage(image))
        writer.Close()

        const reader = testsUtils.openReader(filePath)
        assert.equal(Number(reader.GetImage2DCount()), 1)
        const h = reader.GetImage(0).GetHeader()
        assert.equal(Number(h.pose.rotation.w), 1.0)
        assert.equal(Number(h.pose.rotation.x), 0.0)
        assert.equal(Number(h.pose.rotation.y), 0.0)
        assert.equal(Number(h.pose.rotation.z), 0.0)
    })

    it('AddScan and AddImage combined', async () => {
        const filePath = path.join(OUTPUT_DIR, 'AsyncScanAndImage.e57')
        const writer = new E57Writer(filePath)

        const scanIdx = await testsUtils.withKeepAlive(writer.AddScan(testsUtils.makeCartesianHeader('Combined Scan GUID'), testsUtils.makePoints(32)))
        assert.equal(Number(scanIdx), 0)

        const image = new E57WriterImage(
            IMAGE_PATH,
            E57.LibE57.Image2DType.ImageJPEG,
            E57.LibE57.Image2DProjection.ProjectionVisual
        )
        const bytes = await testsUtils.withKeepAlive(writer.AddImage(image))
        assert.ok(Number(bytes) > 0)

        writer.Close()
        assert.ok(fs.existsSync(filePath))

        const reader = testsUtils.openReader(filePath)
        assert.equal(Number(reader.GetData3DCount()),  1)
        assert.equal(Number(reader.GetImage2DCount()), 1)

        const h = reader.GetScan(0).GetHeader()
        assert.equal(Number(h.pointCount), 32)
        const pts = await testsUtils.withKeepAlive(reader.GetScan(0).ReadScan())
        assert.equal(Number(pts.get(0).cartesianX),  0)
        assert.equal(Number(pts.get(31).cartesianX), 31)
        assert.equal(Number(pts.get(31).cartesianY), 31)
        assert.equal(Number(pts.get(31).cartesianZ), 31)

        assert.equal(Number(reader.GetImage(0).GetHeader().imageSize), Number(bytes))
        const imgData = await testsUtils.withKeepAlive(reader.GetImage(0).ReadImage())
        assert.equal(imgData.byteLength, Number(bytes))
    })
})

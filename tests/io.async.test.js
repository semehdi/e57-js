import { describe, it, before, after } from 'node:test'
import assert from 'node:assert/strict'
import fs from 'node:fs'
import path from 'node:path'
import { fileURLToPath } from 'node:url'
import { E57, E57Reader, E57Writer, E57WriterImage } from '../dist/index.mjs'
import * as testsUtils from './utils.js'

const __dirname = path.dirname(fileURLToPath(import.meta.url))
const OUTPUT_DIR = path.join(__dirname, 'output')
const IMAGE_PATH   = path.join(__dirname, 'data/images/image_1.jpg')
const IMAGE_WIDTH  = 1960
const IMAGE_HEIGHT = 980

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
        const idx = await testsUtils.withKeepAlive(writer.AddScan(testsUtils.makeCartesianHeader('Async Scan GUID'), testsUtils.makePoints(50000)))
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

    it('AddImage header fields round-trip', async () => {
        const filePath = path.join(OUTPUT_DIR, 'ImageHeader.e57')
        const writer = new E57Writer(filePath)
        const image = new E57WriterImage(
            IMAGE_PATH,
            E57.LibE57.Image2DType.ImageJPEG,
            E57.LibE57.Image2DProjection.ProjectionVisual
        )

        image.setName('Front camera')
        image.setGuid('Image-GUID-001')
        image.setRotation(1.0, 0.0, 0.0, 0.0)
        image.setTrasnlation(1.0, 2.0, 3.0)

        const h = image.getHeader()
        h.description                          = 'Test image description'
        h.sensorVendor                         = 'e57-js'
        h.sensorModel                          = 'CamX-9000'
        h.sensorSerialNumber                   = 'SN-123456'
        h.associatedData3DGuid                 = 'Scan-GUID-001'
        h.setAcquisitionDateTime(1748822400.0, 0)

        const bytes = await testsUtils.withKeepAlive(writer.AddImage(image))
        writer.Close()

        const reader = testsUtils.openReader(filePath)
        assert.equal(Number(reader.GetImage2DCount()), 1)

        const rh = reader.GetImage(0).GetHeader()

        // string fields
        assert.equal(rh.name,                 'Front camera')
        assert.equal(rh.guid,                 'Image-GUID-001')
        assert.equal(rh.description,          'Test image description')
        assert.equal(rh.sensorVendor,         'e57-js')
        assert.equal(rh.sensorModel,          'CamX-9000')
        assert.equal(rh.sensorSerialNumber,   'SN-123456')
        assert.equal(rh.associatedData3DGuid, 'Scan-GUID-001')

        // acquisition date-time
        assert.equal(Number(rh.acquisitionDateTime.dateTimeValue), 1748822400.0)
        assert.equal(Number(rh.acquisitionDateTime.isAtomicClockReferenced), 0)

        // top-level numeric fields
        assert.equal(Number(rh.width),           IMAGE_WIDTH)
        assert.equal(Number(rh.height),          IMAGE_HEIGHT)
        assert.equal(Number(rh.imageSize),       Number(bytes))
        assert.equal(Number(rh.imageType),       Number(E57.LibE57.Image2DType.ImageJPEG))
        assert.equal(Number(rh.imageVisualType), Number(E57.LibE57.Image2DType.ImageJPEG))
        assert.equal(Number(rh.imageMaskType),   Number(E57.LibE57.Image2DType.ImageNone))
        assert.equal(Number(rh.imageProjection), Number(E57.LibE57.Image2DProjection.ProjectionVisual))

        // visual reference representation (populated for ProjectionVisual)
        const vr = rh.visualReferenceRepresentation
        assert.equal(Number(vr.imageWidth),    IMAGE_WIDTH)
        assert.equal(Number(vr.imageHeight),   IMAGE_HEIGHT)
        assert.equal(Number(vr.jpegImageSize), Number(bytes))
        assert.equal(Number(vr.pngImageSize),  0)
        assert.equal(Number(vr.imageMaskSize), 0)

        // pinhole representation (not applicable for ProjectionVisual — all zero)
        const pr = rh.pinholeRepresentation
        assert.equal(Number(pr.imageWidth),      0)
        assert.equal(Number(pr.imageHeight),     0)
        assert.equal(Number(pr.jpegImageSize),   0)
        assert.equal(Number(pr.pngImageSize),    0)
        assert.equal(Number(pr.imageMaskSize),   0)
        assert.equal(Number(pr.focalLength),     0)
        assert.equal(Number(pr.pixelWidth),      0)
        assert.equal(Number(pr.pixelHeight),     0)
        assert.equal(Number(pr.principalPointX), 0)
        assert.equal(Number(pr.principalPointY), 0)

        // cylindrical representation (not applicable for ProjectionVisual — all zero)
        const cr = rh.cylindricalRepresentation
        assert.equal(Number(cr.imageWidth),      0)
        assert.equal(Number(cr.imageHeight),     0)
        assert.equal(Number(cr.jpegImageSize),   0)
        assert.equal(Number(cr.pngImageSize),    0)
        assert.equal(Number(cr.imageMaskSize),   0)
        assert.equal(Number(cr.pixelWidth),      0)
        assert.equal(Number(cr.pixelHeight),     0)
        assert.equal(Number(cr.radius),          0)
        assert.equal(Number(cr.principalPointY), 0)

        // pinhole camera distortion extension (defaults — only populated for ProjectionPinhole)
        const dc = rh.pinholeCameraDistortionExt
        assert.equal(Number(dc.cameraNumber), 0)
        assert.equal(Number(dc.CV_K1), 0); assert.equal(Number(dc.CV_K2), 0)
        assert.equal(Number(dc.CV_K3), 0); assert.equal(Number(dc.CV_K4), 0)
        assert.equal(Number(dc.CV_K5), 0); assert.equal(Number(dc.CV_K6), 0)
        assert.equal(Number(dc.CV_P1), 0); assert.equal(Number(dc.CV_P2), 0)
        assert.equal(Number(dc.CV_CX), 0); assert.equal(Number(dc.CV_CY), 0)
        assert.equal(Number(dc.CV_FX), 0); assert.equal(Number(dc.CV_FY), 0)
        assert.equal(Number(dc.CV_WIDTH),  0)
        assert.equal(Number(dc.CV_HEIGHT), 0)

        // pose
        assert.equal(Number(rh.pose.rotation.w),    1.0)
        assert.equal(Number(rh.pose.rotation.x),    0.0)
        assert.equal(Number(rh.pose.rotation.y),    0.0)
        assert.equal(Number(rh.pose.rotation.z),    0.0)
        assert.equal(Number(rh.pose.translation.x), 1.0)
        assert.equal(Number(rh.pose.translation.y), 2.0)
        assert.equal(Number(rh.pose.translation.z), 3.0)
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

    it('ReadScan from file', async () => {
        const filePath = path.join(OUTPUT_DIR, 'AsyncReadScanFromFile.e57')
        const numPoints = 64
        const writer = new E57Writer(filePath)
        await testsUtils.withKeepAlive(writer.AddScan(testsUtils.makeCartesianHeader('ReadScan File GUID'), testsUtils.makePoints(numPoints)))
        writer.Close()

        const reader = new E57Reader(filePath)

        assert.equal(Number(reader.GetData3DCount()), 1)
        assert.equal(reader.GetScan(0).GetHeader().guid, 'ReadScan File GUID')
        assert.equal(Number(reader.GetScan(0).GetHeader().pointCount), numPoints)

        const pts = await testsUtils.withKeepAlive(reader.GetScan(0).ReadScan())
        for (let i = 0; i < numPoints; i++) {
            const pt = pts.get(i)
            assert.equal(Number(pt.cartesianX), i)
            assert.equal(Number(pt.cartesianY), i)
            assert.equal(Number(pt.cartesianZ), i)
        }
    })

    it('FromBuffer', async () => {
        const numPoints = 64
        const writer    = E57Writer.ToBuffer()

        await testsUtils.withKeepAlive(writer.AddScan(testsUtils.makeCartesianHeader('FromBuffer Async GUID'), testsUtils.makePoints(numPoints)))

        const imgBuf   = new Uint8Array(fs.readFileSync(IMAGE_PATH))
        const image    = E57WriterImage.FromBuffer(imgBuf, E57.LibE57.Image2DType.ImageJPEG, E57.LibE57.Image2DProjection.ProjectionVisual)
        image.setName('FromBuffer image')
        const imageBytes = await testsUtils.withKeepAlive(writer.AddImage(image))
        assert.ok(Number(imageBytes) > 0)

        const buffer = writer.Close()
        assert.ok(buffer instanceof Uint8Array)
        assert.ok(buffer.byteLength > 0)

        const reader = E57Reader.FromBuffer(buffer)

        assert.equal(Number(reader.GetData3DCount()), 1)
        assert.equal(reader.GetScan(0).GetHeader().guid, 'FromBuffer Async GUID')
        assert.equal(Number(reader.GetScan(0).GetHeader().pointCount), numPoints)

        const pts = await testsUtils.withKeepAlive(reader.GetScan(0).ReadScan())
        for (let i = 0; i < numPoints; i++) {
            const pt = pts.get(i)
            assert.equal(Number(pt.cartesianX), i)
            assert.equal(Number(pt.cartesianY), i)
            assert.equal(Number(pt.cartesianZ), i)
        }

        assert.equal(Number(reader.GetImage2DCount()), 1)
        assert.equal(reader.GetImage(0).GetHeader().name, 'FromBuffer image')

        const imgData = await testsUtils.withKeepAlive(reader.GetImage(0).ReadImage())
        assert.equal(imgData.byteLength, Number(imageBytes))
        reader.Close();
    })
})

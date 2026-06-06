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

// --- tests ------------------------------------------------------------------

describe('SimpleWriter', () => {
    before(async () => {
        await E57.Init()
        fs.mkdirSync(OUTPUT_DIR, { recursive: true })
    })

    after(() => {
        fs.rmSync(OUTPUT_DIR, { recursive: true, force: true })
    })

    it('PathError', () => {
        assert.throws(() => new E57Writer('./no-path/empty.e57'))
    })

    it('Empty', () => {
        const filePath = path.join(OUTPUT_DIR, 'empty.e57')
        new E57Writer(filePath).Close()

        const reader = testsUtils.openReader(filePath)
        assert.equal(Number(reader.GetData3DCount()),0)
        assert.equal(Number(reader.GetImage2DCount()),0)
    })

    it('ZeroPoints', () => {
        const filePath = path.join(OUTPUT_DIR, 'ZeroPoints.e57')
        const writer = new E57Writer(filePath)
        const header = new E57.LibE57.Data3D()
        header.guid = 'Zero Points Header GUID'
        header.pointFields.cartesianXField = true
        header.pointFields.cartesianYField = true
        header.pointFields.cartesianZField = true
        header.cartesianBounds.xMinimum = 0.0
        writer.AddScanSync(header, [])
        writer.Close()

        const reader = testsUtils.openReader(filePath)
        assert.equal(Number(reader.GetData3DCount()),1)
        const h = reader.GetScan(0).GetHeader()
        assert.equal(h.guid, 'Zero Points Header GUID')
        assert.equal(Number(h.pointCount), 0)
    })

    it('CartesianPoints', () => {
        const filePath = path.join(OUTPUT_DIR, 'CartesianPoints.e57')
        const numPoints = 1025
        const writer = new E57Writer(filePath)
        const header = new E57.LibE57.Data3D()
        header.guid = 'Cartesian Points Header GUID'
        header.pointFields.cartesianXField = true
        header.pointFields.cartesianYField = true
        header.pointFields.cartesianZField = true

        const points = Array.from({ length: numPoints }, (_, i) => {
            const pt = new E57.LibE57.Point()
            pt.cartesianX = i; pt.cartesianY = i; pt.cartesianZ = i
            return pt
        })

        writer.AddScanSync(header, points)
        writer.Close()

        const reader = testsUtils.openReader(filePath)
        assert.equal(Number(reader.GetData3DCount()),1)
        const h = reader.GetScan(0).GetHeader()
        assert.equal(Number(h.pointCount), numPoints)

        const read = reader.GetScan(0).ReadScanSync()
        const first = read.get(0)
        assert.equal(Number(first.cartesianX), 0)
        assert.equal(Number(first.cartesianY), 0)
        assert.equal(Number(first.cartesianZ), 0)
        const last = read.get(numPoints - 1)
        assert.equal(Number(last.cartesianX), numPoints - 1)
        assert.equal(Number(last.cartesianY), numPoints - 1)
        assert.equal(Number(last.cartesianZ), numPoints - 1)
    })

    it('ColouredCartesianPoints', () => {
        const filePath = path.join(OUTPUT_DIR, 'ColouredCartesianPoints.e57')
        const numPoints = 1025
        const writer = new E57Writer(filePath)
        const header = testsUtils.makeColouredCartesianHeader()
        header.guid = 'Coloured Cartesian Points Header GUID'

        const points = Array.from({ length: numPoints }, (_, i) => {
            const pt = new E57.LibE57.Point()
            pt.cartesianX = i; pt.cartesianY = i; pt.cartesianZ = i
            pt.colorRed = 0; pt.colorGreen = 0; pt.colorBlue = 255
            return pt
        })

        writer.AddScanSync(header, points)
        writer.Close()

        const reader = testsUtils.openReader(filePath)
        assert.equal(Number(reader.GetData3DCount()),1)
        const read = reader.GetScan(0).ReadScanSync()
        const pt = read.get(0)
        assert.equal(Number(pt.colorRed),   0)
        assert.equal(Number(pt.colorGreen), 0)
        assert.equal(Number(pt.colorBlue),  255)
    })

    it('ColouredCartesianPoints 16-bit', () => {
        const filePath = path.join(OUTPUT_DIR, 'ColouredCartesianPoints16bit.e57')
        const numPoints = 64
        const MAX16 = 65535

        const writer = new E57Writer(filePath)
        const header = testsUtils.make16BitColouredCartesianHeader()
        header.guid = 'Coloured Cartesian Points 16-bit Header GUID'

        const points = Array.from({ length: numPoints }, (_, i) => {
            const pt = new E57.LibE57.Point()
            pt.cartesianX = i; pt.cartesianY = i; pt.cartesianZ = i
            pt.colorRed   = Math.trunc(i * 1024)
            pt.colorGreen = Math.trunc(MAX16 - i * 1024)
            pt.colorBlue  = Math.trunc(Math.trunc(MAX16 / 2))
            return pt
        })

        writer.AddScanSync(header, points)
        writer.Close()

        const reader = testsUtils.openReader(filePath)
        assert.equal(Number(reader.GetData3DCount()), 1)

        const h = reader.GetScan(0).GetHeader()
        assert.equal(Number(h.colorLimits.colorRedMaximum),   MAX16)
        assert.equal(Number(h.colorLimits.colorGreenMaximum), MAX16)
        assert.equal(Number(h.colorLimits.colorBlueMaximum),  MAX16)

        const read = reader.GetScan(0).ReadScanSync()
        const first = read.get(0)
        assert.equal(Number(first.colorRed),   0)
        assert.equal(Number(first.colorGreen), MAX16)
        assert.equal(Number(first.colorBlue),  Math.trunc(MAX16 / 2))

        const last = read.get(numPoints - 1)
        assert.equal(Number(last.colorRed),   Math.trunc((numPoints - 1) * 1024))
        assert.equal(Number(last.colorGreen), Math.trunc(MAX16 - (numPoints - 1) * 1024))
        assert.equal(Number(last.colorBlue),  Math.trunc(MAX16 / 2))
    })

    it('ColouredCubeScaledInt', () => {
        const filePath = path.join(OUTPUT_DIR, 'ColouredCubeScaledInt.e57')
        const random = testsUtils.createRandom(42)
        const writer = new E57Writer(filePath)
        const header = testsUtils.makeColouredCartesianHeader()
        header.guid        = 'Cube Scaled Int Scan Header GUID'
        header.description = 'e57-js test: cube of coloured points using scaled integers'
        header.pointFields.pointRangeNodeType = E57.LibE57.NumericalNodeType.ScaledInteger
        header.pointFields.pointRangeScale    = 0.001

        const points = []
        testsUtils.generateCubePoints(1.0, 1280, random, (face, [x, y, z]) => {
            const pt = new E57.LibE57.Point()
            pt.cartesianX = x; pt.cartesianY = y; pt.cartesianZ = z
            ;[pt.colorRed, pt.colorGreen, pt.colorBlue] = testsUtils.FACE_COLORS[face]
            points.push(pt)
        })

        writer.AddScanSync(header, points)
        writer.Close()

        const reader = testsUtils.openReader(filePath)
        assert.equal(Number(reader.GetData3DCount()),1)
        const h = reader.GetScan(0).GetHeader()
        assert.equal(Number(h.pointCount), 1280 * 6)
        assert.equal(Number(h.pointFields.pointRangeNodeType), Number(E57.LibE57.NumericalNodeType.ScaledInteger))
        assert.equal(Number(h.pointFields.pointRangeScale), 0.001)
    })

    it('GeorefScaledInt', () => {
        // Scanner positioned at the city of Paris in EPSG:3857 (Web Mercator, meters)
        const ORIGIN_X = 261848.4   // easting
        const ORIGIN_Y = 6250566.5  // northing
        const ORIGIN_Z = 35.0       // elevation above ground (m)

        const filePath = path.join(OUTPUT_DIR, 'GeorefScaledInt.e57')
        const numPoints = 128
        const writer = new E57Writer(filePath)
        const header = new E57.LibE57.Data3D()
        header.guid = 'Georef Scaled Int Header GUID'
        header.pointFields.cartesianXField    = true
        header.pointFields.cartesianYField    = true
        header.pointFields.cartesianZField    = true
        header.pointFields.pointRangeNodeType = E57.LibE57.NumericalNodeType.ScaledInteger
        header.pointFields.pointRangeScale    = 0.001

        // identity rotation — EPSG:3857 axes align with the scanner axes
        header.pose.rotation.w = 1.0
        header.pose.rotation.x = 0.0
        header.pose.rotation.y = 0.0
        header.pose.rotation.z = 0.0
        header.pose.translation.x = ORIGIN_X
        header.pose.translation.y = ORIGIN_Y
        header.pose.translation.z = ORIGIN_Z

        // local points relative to the scanner position
        const points = Array.from({ length: numPoints }, (_, i) => {
            const pt = new E57.LibE57.Point()
            pt.cartesianX = i * 0.25
            pt.cartesianY = i * 0.10
            pt.cartesianZ = i * 0.05
            return pt
        })

        writer.AddScanSync(header, points)
        writer.Close()

        const reader = testsUtils.openReader(filePath)
        assert.equal(Number(reader.GetData3DCount()), 1)

        const h = reader.GetScan(0).GetHeader()
        assert.equal(Number(h.pointCount), numPoints)
        assert.equal(Number(h.pointFields.pointRangeNodeType), Number(E57.LibE57.NumericalNodeType.ScaledInteger))
        assert.equal(Number(h.pointFields.pointRangeScale), 0.001)

        assert.equal(Number(h.pose.rotation.w), 1.0)
        assert.equal(Number(h.pose.rotation.x), 0.0)
        assert.equal(Number(h.pose.rotation.y), 0.0)
        assert.equal(Number(h.pose.rotation.z), 0.0)
        assert.equal(Number(h.pose.translation.x), ORIGIN_X)
        assert.equal(Number(h.pose.translation.y), ORIGIN_Y)
        assert.equal(Number(h.pose.translation.z), ORIGIN_Z)

        // apply pose to every point and verify global EPSG:3857 coordinates
        const pts = reader.GetScan(0).ReadScanSync()
        for (let i = 0; i < numPoints; i++) {
            const pt = pts.get(i)
            assert.ok(Math.abs(Number(pt.cartesianX) - (ORIGIN_X + i * 0.25)) < 1e-5)
            assert.ok(Math.abs(Number(pt.cartesianY) - (ORIGIN_Y + i * 0.10)) < 1e-5)
            assert.ok(Math.abs(Number(pt.cartesianZ) - (ORIGIN_Z + i * 0.05)) < 1e-5)
        }

        // Do not apply pose to every point and verify global EPSG:3857 coordinates
        const ptsNoTransform = reader.GetScan(0).ReadScanSync(false)
        for (let i = 0; i < numPoints; i++) {
            const pt = ptsNoTransform.get(i)
            assert.ok(Math.abs(Number(pt.cartesianX) - i * 0.25) < 1e-5)
            assert.ok(Math.abs(Number(pt.cartesianY) - i * 0.10) < 1e-5)
            assert.ok(Math.abs(Number(pt.cartesianZ) - i * 0.05) < 1e-5)
        }
    })

    it('GeorefSpherical', () => {
        // Scanner positioned at Paris in EPSG:3857 (Web Mercator, meters)
        const ORIGIN_X = 261848.4
        const ORIGIN_Y = 6250566.5
        const ORIGIN_Z = 35.0

        const filePath = path.join(OUTPUT_DIR, 'GeorefSpherical.e57')
        const numPoints = 128
        const writer = new E57Writer(filePath)
        const header = new E57.LibE57.Data3D()
        header.guid = 'Georef Scaled Int Spherical Header GUID'
        header.pointFields.sphericalRangeField     = true
        header.pointFields.sphericalAzimuthField   = true
        header.pointFields.sphericalElevationField = true

        // identity rotation — EPSG:3857 axes align with the scanner axes
        header.pose.rotation.w = 1.0
        header.pose.rotation.x = 0.0
        header.pose.rotation.y = 0.0
        header.pose.rotation.z = 0.0
        header.pose.translation.x = ORIGIN_X
        header.pose.translation.y = ORIGIN_Y
        header.pose.translation.z = ORIGIN_Z

        // local Cartesian offsets converted to spherical for storage
        const points = Array.from({ length: numPoints }, (_, i) => {
            const pt = new E57.LibE57.Point()
            pt.cartesianX = (i + 1) * 0.25
            pt.cartesianY = (i + 1) * 0.10
            pt.cartesianZ = (i + 1) * 0.05
            pt.cartesianToSpherical()
            return pt
        })

        writer.AddScanSync(header, points)
        writer.Close()

        const reader = testsUtils.openReader(filePath)
        assert.equal(Number(reader.GetData3DCount()), 1)

        const h = reader.GetScan(0).GetHeader()
        assert.equal(Number(h.pointCount), numPoints)
        assert.ok(h.pointFields.sphericalRangeField)
        assert.ok(h.pointFields.sphericalAzimuthField)
        assert.ok(h.pointFields.sphericalElevationField)

        assert.equal(Number(h.pose.rotation.w), 1.0)
        assert.equal(Number(h.pose.rotation.x), 0.0)
        assert.equal(Number(h.pose.rotation.y), 0.0)
        assert.equal(Number(h.pose.rotation.z), 0.0)
        assert.equal(Number(h.pose.translation.x), ORIGIN_X)
        assert.equal(Number(h.pose.translation.y), ORIGIN_Y)
        assert.equal(Number(h.pose.translation.z), ORIGIN_Z)

        // with transform: sphericalToCartesian + pose applied → global EPSG:3857 Cartesian
        const pts = reader.GetScan(0).ReadScanSync()
        for (let i = 0; i < numPoints; i++) {
            const pt = pts.get(i)
            // 1 mm tolerance: ScaledInteger (scale=0.001) propagates through spherical→Cartesian
            assert.ok(Math.abs(Number(pt.cartesianX) - (ORIGIN_X + (i + 1) * 0.25)) < 1e-5)
            assert.ok(Math.abs(Number(pt.cartesianY) - (ORIGIN_Y + (i + 1) * 0.10)) < 1e-5)
            assert.ok(Math.abs(Number(pt.cartesianZ) - (ORIGIN_Z + (i + 1) * 0.05)) < 1e-5)
        }

        // without transform: raw spherical values as stored
        const ptsNoTransform = reader.GetScan(0).ReadScanSync(false)
        for (let i = 0; i < numPoints; i++) {
            const pt = ptsNoTransform.get(i)
            assert.ok(Math.abs(Number(pt.sphericalRange)     - points[i].sphericalRange)     < 1e-5)
            assert.ok(Math.abs(Number(pt.sphericalAzimuth)   - points[i].sphericalAzimuth)   < 1e-5)
            assert.ok(Math.abs(Number(pt.sphericalElevation) - points[i].sphericalElevation) < 1e-5)
        }
    })

    it('CartesianPoseRotation', () => {
        // 90° rotation around Z: w = 1/√2, z = 1/√2
        // Rotation matrix: [ 0 -1  0 ]
        //                  [ 1  0  0 ]
        //                  [ 0  0  1 ]
        // → local (x, 0, 0) maps to global (TX, x + TY, TZ)
        const SQRT2_2 = Math.SQRT2 / 2
        const TX = 10.0, TY = 20.0, TZ = 5.0

        const filePath = path.join(OUTPUT_DIR, 'CartesianPoseRotation.e57')
        const numPoints = 64
        const writer = new E57Writer(filePath)
        const header = new E57.LibE57.Data3D()
        header.guid = 'Cartesian Pose Rotation Header GUID'
        header.pointFields.cartesianXField = true
        header.pointFields.cartesianYField = true
        header.pointFields.cartesianZField = true

        header.pose.rotation.w = SQRT2_2
        header.pose.rotation.x = 0.0
        header.pose.rotation.y = 0.0
        header.pose.rotation.z = SQRT2_2
        header.pose.translation.x = TX
        header.pose.translation.y = TY
        header.pose.translation.z = TZ

        // local points along the X axis
        const points = Array.from({ length: numPoints }, (_, i) => {
            const pt = new E57.LibE57.Point()
            pt.cartesianX = i * 0.5
            pt.cartesianY = 0
            pt.cartesianZ = 0
            return pt
        })

        writer.AddScanSync(header, points)
        writer.Close()

        const reader = testsUtils.openReader(filePath)
        assert.equal(Number(reader.GetData3DCount()), 1)

        const h = reader.GetScan(0).GetHeader()
        assert.equal(Number(h.pointCount), numPoints)
        assert.ok(Math.abs(Number(h.pose.rotation.w) - SQRT2_2) < 1e-10)
        assert.ok(Math.abs(Number(h.pose.rotation.z) - SQRT2_2) < 1e-10)
        assert.equal(Number(h.pose.translation.x), TX)
        assert.equal(Number(h.pose.translation.y), TY)
        assert.equal(Number(h.pose.translation.z), TZ)

        // with transform: local X rotates into global Y
        const pts = reader.GetScan(0).ReadScanSync()
        for (let i = 0; i < numPoints; i++) {
            const pt = pts.get(i)
            assert.ok(Math.abs(Number(pt.cartesianX) - TX)              < 1e-5)
            assert.ok(Math.abs(Number(pt.cartesianY) - (i * 0.5 + TY)) < 1e-5)
            assert.ok(Math.abs(Number(pt.cartesianZ) - TZ)              < 1e-5)
        }

        // without transform: raw local Cartesian values
        const ptsNoTransform = reader.GetScan(0).ReadScanSync(false)
        for (let i = 0; i < numPoints; i++) {
            const pt = ptsNoTransform.get(i)
            assert.ok(Math.abs(Number(pt.cartesianX) - i * 0.5) < 1e-5)
            assert.ok(Math.abs(Number(pt.cartesianY))            < 1e-5)
            assert.ok(Math.abs(Number(pt.cartesianZ))            < 1e-5)
        }
    })

    it('ScanPoints chunk-by-chunk', () => {
        const filePath = path.join(OUTPUT_DIR, 'ChunkRead.e57')
        const numPoints = 1024
        const chunkSize = 50
        const writer = new E57Writer(filePath)
        const header = new E57.LibE57.Data3D()
        header.guid = 'Chunk Read Header GUID'
        header.pointFields.cartesianXField = true
        header.pointFields.cartesianYField = true
        header.pointFields.cartesianZField = true

        const points = Array.from({ length: numPoints }, (_, i) => {
            const pt = new E57.LibE57.Point()
            pt.cartesianX = i; pt.cartesianY = i; pt.cartesianZ = i
            return pt
        })

        writer.AddScanSync(header, points)
        writer.Close()

        const reader = testsUtils.openReader(filePath)
        const scan = reader.GetScan(0)
        assert.equal(Number(scan.GetHeader().pointCount), numPoints)

        let totalRead = 0
        let expectedIndex = 0

        scan.ScanPoints(chunkSize, (chunk) => {
            const size = Number(chunk.size())
            assert.ok(size > 0)
            assert.ok(size <= chunkSize)

            for (let i = 0; i < size; i++) {
                const pt = chunk.get(i)
                assert.equal(Number(pt.cartesianX), expectedIndex)
                assert.equal(Number(pt.cartesianY), expectedIndex)
                assert.equal(Number(pt.cartesianZ), expectedIndex)
                expectedIndex++
            }

            totalRead += size
        })

        assert.equal(totalRead, numPoints)
    })

    it('MultipleScans', () => {
        const filePath = path.join(OUTPUT_DIR, 'MultipleScans.e57')
        const writer = new E57Writer(filePath)
        const header = new E57.LibE57.Data3D()
        header.pointFields.cartesianXField = true
        header.pointFields.cartesianYField = true
        header.pointFields.cartesianZField = true

        const makeScan = (cubeSize) => {
            const pts = []
            testsUtils.generateCubeCornerPoints(cubeSize, ([x, y, z]) => {
                const pt = new E57.LibE57.Point()
                pt.cartesianX = x; pt.cartesianY = y; pt.cartesianZ = z
                pts.push(pt)
            })
            return pts
        }

        header.guid = 'Multiple Scans Scan 1 Header GUID'
        writer.AddScanSync(header, makeScan(1.0))

        header.guid = 'Multiple Scans Scan 2 Header GUID'
        writer.AddScanSync(header, makeScan(0.5))

        writer.Close()

        const reader = testsUtils.openReader(filePath)
        assert.equal(Number(reader.GetData3DCount()),2)
        assert.equal(reader.GetScan(0).GetHeader().guid, 'Multiple Scans Scan 1 Header GUID')
        assert.equal(Number(reader.GetScan(0).GetHeader().pointCount), 8)
        assert.equal(reader.GetScan(1).GetHeader().guid, 'Multiple Scans Scan 2 Header GUID')
        assert.equal(Number(reader.GetScan(1).GetHeader().pointCount), 8)
    })

    it('SphericalCubePoints', () => {
        const filePath = path.join(OUTPUT_DIR, 'SphericalCubePoints.e57')
        const random = testsUtils.createRandom(42)
        const writer = new E57Writer(filePath)
        const header = new E57.LibE57.Data3D()
        header.guid = 'Spherical Cube Points Header GUID'
        header.pointFields.sphericalRangeField     = true
        header.pointFields.sphericalAzimuthField   = true
        header.pointFields.sphericalElevationField = true

        const points = []
        testsUtils.generateSphericalCubePoints(1.0, 1280, random, (_, { range, azimuth, elevation }) => {
            const pt = new E57.LibE57.Point()
            pt.sphericalRange     = range
            pt.sphericalAzimuth   = azimuth
            pt.sphericalElevation = elevation
            points.push(pt)
        })

        writer.AddScanSync(header, points)
        writer.Close()

        const reader = testsUtils.openReader(filePath)
        assert.equal(Number(reader.GetData3DCount()), 1)
        const h = reader.GetScan(0).GetHeader()
        assert.equal(Number(h.pointCount), 1280 * 6)
        assert.ok(h.pointFields.sphericalRangeField)
        assert.ok(h.pointFields.sphericalAzimuthField)
        assert.ok(h.pointFields.sphericalElevationField)

        const read = reader.GetScan(0).ReadScanSync()
        const first = read.get(0)
        assert.ok(Number(first.sphericalRange) > 0)
        assert.ok(Math.abs(Number(first.sphericalRange)     - points[0].sphericalRange)     < 1e-5)
        assert.ok(Math.abs(Number(first.sphericalAzimuth)   - points[0].sphericalAzimuth)   < 1e-5)
        assert.ok(Math.abs(Number(first.sphericalElevation) - points[0].sphericalElevation) < 1e-5)
    })

    it('AddImageSync header fields round-trip', () => {
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

        const bytes = writer.AddImageSync(image, IMAGE_WIDTH, IMAGE_HEIGHT)
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

    it('AddImageSync multiple images', () => {
        const filePath = path.join(OUTPUT_DIR, 'MultipleImages.e57')
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

        writer.AddImageSync(makeImage('Camera 1'), IMAGE_WIDTH, IMAGE_HEIGHT)
        writer.AddImageSync(makeImage('Camera 2'), IMAGE_WIDTH, IMAGE_HEIGHT)
        writer.AddImageSync(makeImage('Camera 3'), IMAGE_WIDTH, IMAGE_HEIGHT)
        writer.Close()

        const reader = testsUtils.openReader(filePath)
        assert.equal(Number(reader.GetImage2DCount()), 3)
        assert.equal(reader.GetImage(0).GetHeader().name, 'Camera 1')
        assert.equal(reader.GetImage(1).GetHeader().name, 'Camera 2')
        assert.equal(reader.GetImage(2).GetHeader().name, 'Camera 3')
    })

    it('AddImageSync bytes match source file size', () => {
        const filePath = path.join(OUTPUT_DIR, 'ImageBytes.e57')
        const writer = new E57Writer(filePath)
        const image = new E57WriterImage(
            IMAGE_PATH,
            E57.LibE57.Image2DType.ImageJPEG,
            E57.LibE57.Image2DProjection.ProjectionVisual
        )

        const bytes = writer.AddImageSync(image, IMAGE_WIDTH, IMAGE_HEIGHT)
        writer.Close()

        const sourceSize = fs.statSync(IMAGE_PATH).size
        assert.equal(Number(bytes), sourceSize)
    })

    it('AddImageSync with rotation and translation', () => {
        const filePath = path.join(OUTPUT_DIR, 'ImagePose.e57')
        const writer = new E57Writer(filePath)
        const image = new E57WriterImage(
            IMAGE_PATH,
            E57.LibE57.Image2DType.ImageJPEG,
            E57.LibE57.Image2DProjection.ProjectionVisual
        )
        image.setName('Posed camera')
        image.setRotation(1.0, 0.0, 0.0, 0.0)
        image.setTrasnlation(1.5, 2.5, 3.5)
        writer.AddImageSync(image, IMAGE_WIDTH, IMAGE_HEIGHT)
        writer.Close()

        const reader = testsUtils.openReader(filePath)
        assert.equal(Number(reader.GetImage2DCount()), 1)
        const pose = reader.GetImage(0).GetHeader().pose
        assert.equal(Number(pose.rotation.w), 1.0)
        assert.equal(Number(pose.rotation.x), 0.0)
        assert.equal(Number(pose.rotation.y), 0.0)
        assert.equal(Number(pose.rotation.z), 0.0)
        assert.equal(Number(pose.translation.x), 1.5)
        assert.equal(Number(pose.translation.y), 2.5)
        assert.equal(Number(pose.translation.z), 3.5)
    })

    it('ChineseFileName', () => {
        const filePath = path.join(OUTPUT_DIR, '测试点云.e57')
        new E57Writer(filePath).Close()

        const reader = testsUtils.openReader(filePath)
        assert.equal(Number(reader.GetData3DCount()),0)
    })

    it('UmlautFileName', () => {
        const filePath = path.join(OUTPUT_DIR, 'test filename äöü.e57')
        new E57Writer(filePath).Close()

        const reader = testsUtils.openReader(filePath)
        assert.equal(Number(reader.GetData3DCount()),0)
    })
})

import fs from 'node:fs'
import path from 'node:path'
import assert from 'node:assert/strict'
import { E57, E57Reader, E57Writer } from '../dist/index.mjs'

export function createRandom(seed) {
    let s = seed >>> 0
    return () => {
        s = (Math.imul(s, 1664525) + 1013904223) >>> 0
        return s / 0x100000000
    }
}

const CUBE_CORNERS = [
    [-0.5, -0.5, -0.5], [ 0.5, -0.5, -0.5], [ 0.5,  0.5, -0.5],
    [-0.5,  0.5, -0.5], [-0.5,  0.5,  0.5], [ 0.5,  0.5,  0.5],
    [ 0.5, -0.5,  0.5], [-0.5, -0.5,  0.5],
]

export function generateCubeCornerPoints(cubeSize, cb) {
    for (const c of CUBE_CORNERS) cb(c.map(v => v * cubeSize))
}

// Each face: which axis is fixed, its sign, and the two axes that get random values
const CUBE_FACES = [
    { fixed: 0, sign: -1, rand: [1, 2] }, // -X
    { fixed: 1, sign: -1, rand: [0, 2] }, // -Y
    { fixed: 2, sign: -1, rand: [0, 1] }, // -Z
    { fixed: 0, sign:  1, rand: [1, 2] }, // +X
    { fixed: 1, sign:  1, rand: [0, 2] }, // +Y
    { fixed: 2, sign:  1, rand: [0, 1] }, // +Z
]

export function generateCubePoints(cubeSize, pointsPerFace, random, cb) {
    const half = cubeSize / 2
    for (let face = 0; face < CUBE_FACES.length; face++) {
        const { fixed, sign, rand } = CUBE_FACES[face]
        for (let i = 0; i < pointsPerFace; i++) {
            const p = [0, 0, 0]
            p[fixed]   = sign * half
            p[rand[0]] = random() * cubeSize - half
            p[rand[1]] = random() * cubeSize - half
            cb(face, p)
        }
    }
}

export const FACE_COLORS = [
    [0, 0, 255], [0, 255, 0], [255, 0, 0],
    [0, 0, 255], [0, 255, 0], [255, 0, 0],
]

export function makeColouredCartesianHeader() {
    const h = new E57.LibE57.Data3D()
    h.pointFields.cartesianXField   = true
    h.pointFields.cartesianYField   = true
    h.pointFields.cartesianZField   = true
    h.pointFields.colorRedField     = true
    h.pointFields.colorGreenField   = true
    h.pointFields.colorBlueField    = true
    h.colorLimits.colorRedMaximum   = 255
    h.colorLimits.colorGreenMaximum = 255
    h.colorLimits.colorBlueMaximum  = 255
    return h
}

export function make16BitColouredCartesianHeader() {
    const h = new E57.LibE57.Data3D()
    h.pointFields.cartesianXField   = true
    h.pointFields.cartesianYField   = true
    h.pointFields.cartesianZField   = true
    h.pointFields.colorRedField     = true
    h.pointFields.colorGreenField   = true
    h.pointFields.colorBlueField    = true
    h.colorLimits.colorRedMaximum   = 65535
    h.colorLimits.colorGreenMaximum = 65535
    h.colorLimits.colorBlueMaximum  = 65535
    return h
}

export function openReader(filePath) {
    assert.ok(fs.existsSync(filePath), `file not created: ${filePath}`)
    return new E57Reader(filePath)
}

export function makeCartesianHeader(guid) {
    const header = new E57.LibE57.Data3D()
    header.guid = guid
    header.pointFields.cartesianXField = true
    header.pointFields.cartesianYField = true
    header.pointFields.cartesianZField = true
    return header
}

// Generates cube face points in spherical coordinates (range, azimuth, elevation).
// Cartesian cube points are converted using the standard physics convention:
//   range     = sqrt(x²+y²+z²)
//   azimuth   = atan2(y, x)
//   elevation = atan2(z, sqrt(x²+y²))
export function generateSphericalCubePoints(cubeSize, pointsPerFace, random, cb) {
    generateCubePoints(cubeSize, pointsPerFace, random, (face, [x, y, z]) => {
        const range     = Math.sqrt(x * x + y * y + z * z)
        const azimuth   = Math.atan2(y, x)
        const elevation = Math.atan2(z, Math.sqrt(x * x + y * y))
        cb(face, { range, azimuth, elevation })
    })
}

export function makePoints(count) {
    return Array.from({ length: count }, (_, i) => {
        const pt = new E57.LibE57.Point()
        pt.cartesianX = i; pt.cartesianY = i; pt.cartesianZ = i
        return pt
    })
}

// Keeps the event loop alive while a WASM pthread posts its result back to the
// main thread. Without an active handle, node:test can drain the loop before
// the worker message arrives, leaving the promise permanently pending.
export async function withKeepAlive(promise) {
    const timer = setInterval(() => {}, 50)
    return promise.finally(() => clearInterval(timer))
}
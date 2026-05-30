# e57-js

Node.js library for reading and writing **E57 point cloud files**. The core is a C++ implementation built on [libE57Format](https://github.com/asmaloney/libE57Format), compiled to WebAssembly via Emscripten and wrapped in a JavaScript API.

---

## Installation

```sh
npm install e57-js
```

> **Requires Node.js 18+** and a platform that supports `SharedArrayBuffer` (enabled by default in Node.js).

---

## Quick start

```js
import { E57, E57Reader, E57Writer } from 'e57-js'

// Must be awaited before creating any reader or writer
await E57.Init()
```

---

## Reading

### Open a file

```js
const reader = new E57Reader('scan.e57')
```

### File-level header

```js
const header = reader.GetHeader()  // E57Root
console.log(header.guid, header.creationDateTime)
console.log(`${reader.GetData3DCount()} scans, ${reader.GetImage2DCount()} images`)
```

### Read points — async (non-blocking)

```js
const scan = reader.GetScan(0)
const header = scan.GetHeader()          // Data3D
console.log(header.pointCount)

// Read 1 000 points at a time — returns Promise<VectorPoint>
const chunk = await scan.ReadPoints(1000)
for (let i = 0; i < chunk.size(); i++) {
    const pt = chunk.get(i)
    console.log(pt.cartesianX, pt.cartesianY, pt.cartesianZ)
}
```

### Read points — sync (blocking)

```js
const chunk = scan.ReadPointsSync(1000)  // VectorPoint
```

### Stream a full scan in chunks

```js
// With callback — process and discard each chunk (low memory)
await scan.ScanPoints(1000, (chunk) => {
    for (let i = 0; i < chunk.size(); i++)
        process(chunk.get(i))
})

// Without callback — collect all chunks
const chunks = await scan.ScanPoints(1000)  // VectorPoint[]
```

### Apply scan pose (rotation + translation)

```js
const pose = scan.GetHeader().pose  // RigidBodyTransform
const chunk = await scan.ReadPoints(1000)
for (let i = 0; i < chunk.size(); i++)
    chunk.get(i).transform(pose)    // transforms cartesianX/Y/Z in-place
```

### Read a 2D image — async

```js
const image = reader.GetImage(0)
const header = image.GetHeader()  // ImageHeader
console.log(header.width, header.height, header.imageType)

// Returns Promise<Uint8Array> — zero-copy view into WASM memory
const bytes = await image.ReadImage()

// Save to disk
await image.Save('output')        // extension auto-detected (.jpeg / .png)

// Get as Base64
const b64 = await image.ToBase64()
```

### Read a 2D image — sync

```js
const bytes = image.ReadImageSync()  // Uint8Array
```

---

## Writing

### Create a file

```js
const writer = new E57Writer('output.e57')
```

### Write a 3D scan — async

```js
const header = new E57.LibE57.Data3D()
header.guid = 'my-scan-guid'
header.pointFields.cartesianXField = true
header.pointFields.cartesianYField = true
header.pointFields.cartesianZField = true

const points = []
for (let i = 0; i < 1024; i++) {
    const pt = new E57.LibE57.Point()
    pt.cartesianX = i
    pt.cartesianY = i * 2
    pt.cartesianZ = 0
    points.push(pt)
}

const scanIdx = await writer.AddScan(header, points)  // Promise<number>
```

### Write a 3D scan — sync

```js
const scanIdx = writer.AddScanSync(header, points)    // number
```

### Write a 2D image — async

```js
import { E57WriterImage } from 'e57-js'

const image = new E57WriterImage(
    'photo.jpg',
    E57.LibE57.Image2DType.ImageJPEG,
    E57.LibE57.Image2DProjection.ProjectionVisual
)
image.setName('Front camera')
image.setRotation(1.0, 0.0, 0.0, 0.0)  // unit quaternion (w, x, y, z)

await writer.AddImage(image)  // Promise<number> — bytes written
```

### Write a 2D image — sync

```js
const [width, height] = await image.getDimensions()
writer.AddImageSync(image, width, height)
```

### Close the file

```js
writer.Close()  // flush and finalise — must be called when done
```

---

## Point fields

Each `Point` object exposes the following properties:

| Field | Type | Description |
|---|---|---|
| `cartesianX/Y/Z` | `number` | Cartesian coordinates (metres) |
| `intensity` | `number` | Reflected signal strength |
| `colorRed/Green/Blue` | `number` | RGB colour (0–65535) |
| `normalX/Y/Z` | `number` | Surface normal vector |
| `sphericalRange/Azimuth/Elevation` | `number` | Spherical coordinates |
| `timeStamp` | `number` | Acquisition time |
| `rowIndex/columnIndex` | `number` | Grid position |
| `returnIndex/returnCount` | `number` | Multi-return info |
| `cartesianInvalidState` | `number` | 0 = valid |
| `isColorInvalid` | `number` | 0 = valid |
| `isIntensityInvalid` | `number` | 0 = valid |
| `isTimeStampInvalid` | `number` | 0 = valid |

---

## Stateful scan reading

`ReadPoints` / `ReadPointsSync` maintain an internal cursor per scan. Each call advances the cursor by the number of points read. When the end of the scan is reached, the cursor resets automatically on the next call.

To reset manually:

```js
reader.GetScan(0)  // E57ReaderScan
// internal: reader.reader.ResetScanReader(scanIdx)
```

---

## Build from source

Requires Docker.

```sh
# Build the Docker image with Emscripten
docker build -t e57-js .

# Compile C++ → WASM inside the container
docker run -it -v $(pwd):/app e57-js bash -c "mkdir -p build && cd build && emcmake cmake .. && emmake make"

# Bundle the JavaScript layer
npx tsup
```

The distributable is `dist/` — contains `index.js`, `libe57-js.js`, and `libe57-js.wasm`.

---

## Architecture

```
src/cpp/        C++ source compiled to WASM (reader, writer, point, image)
src/js/         JavaScript wrappers (E57Reader, E57Writer, E57WriterImage)
include/        C++ headers
libE57Format/   Upstream libE57Format (static library)
build/          CMake output (WASM artifacts)
dist/           Final bundle (JS + WASM)
```

All async methods run file I/O on a background `pthread`. The JS Promise resolves on the main thread when the operation completes. The library uses `SharedArrayBuffer` (enabled automatically by Emscripten's `-pthread` flag) for zero-copy image transfers.

# e57-js

JavaScript library for reading and writing **E57 point cloud files**, compatible with **Node.js** and the **browser**.

> **What is a point cloud?** A point cloud is a collection of 3D points in space, typically captured by a laser scanner (LiDAR). Each point has X, Y, Z coordinates and optionally colour, intensity, and other data. E57 is the standard file format for storing them.

---

## Installation

```sh
npm install e57-js
```

Requires **Node.js 18 or higher**, or any modern browser with WebAssembly support.

In the browser, file-system access is not available — use `E57Reader.FromBuffer(buffer)` to read from a `Uint8Array` and `E57Writer.ToBuffer()` to write to one.

---

## Setup

Before doing anything, call `E57.Init()` once and wait for it to finish. This loads the underlying WebAssembly module.

```js
import { E57, E57Reader, E57Writer, E57WriterScan, E57WriterImage } from 'e57-js'

await E57.Init()

// Now you can create readers and writers
```

---

## Reading a file

### Open the file

```js
// From a file path (Node.js)
const reader = new E57Reader('scan.e57')

// From a buffer (browser or Node.js)
const reader = E57Reader.FromBuffer(buffer)  // buffer is a Uint8Array

console.log(reader.GetData3DCount())   // number of 3D scans
console.log(reader.GetImage2DCount())  // number of embedded images
```

### Read the points from a scan

```js
const scan = reader.GetScan(0)  // get the first scan (index 0)

const header = scan.GetHeader()
console.log(`This scan has ${header.pointCount} points`)

// Read all points at once
const points = await scan.ReadScan()

for (let i = 0; i < points.size(); i++) {
    const pt = points.get(i)
    console.log(pt.cartesianX, pt.cartesianY, pt.cartesianZ)
}

// free WASM memory when done
points.delete()
```

### Read in chunks (better for large files)

Reading millions of points all at once can use a lot of memory. Use `ScanPoints` to process them in smaller batches:

```js
await scan.ScanPoints(1000, (chunk) => {
    for (let i = 0; i < chunk.size(); i++) {
        const pt = chunk.get(i)
        // process pt...
    }
    // chunk is freed after this callback returns — do not hold references to it outside
})
```

### Apply the scan pose (position and orientation)

A scan's pose describes where the scanner was placed in the world. When you read with `ReadScan()` the pose is applied automatically — the returned coordinates are already in world space.

Pass `false` to get the raw local coordinates instead:

```js
const worldPoints = await scan.ReadScan()        // world coordinates (default)
const localPoints = await scan.ReadScan(false)   // raw scanner-local coordinates
```

### Read an embedded image

```js
const image = reader.GetImage(0)
const header = image.GetHeader()
console.log(header.width, header.height)

// Get the image as a byte buffer
const bytes = await image.ReadImage()  // Uint8Array
// use bytes...
image.Release(bytes)  // free WASM memory when done

// Or save it directly to disk (extension is auto-detected — memory is freed automatically)
await image.Save('output')   // writes output.jpeg or output.png
```

### Synchronous versions

Every async method has a sync equivalent if you prefer blocking calls:

```js
const points = scan.ReadScanSync()
const bytes  = image.ReadImageSync()
```

> **Exception:** `AddImage` has no sync version.

---

## Writing a file

### Create the file

```js
// To a file path (Node.js)
const writer = new E57Writer('output.e57')

// To a buffer (browser or Node.js) — Close() returns a Uint8Array
const writer = E57Writer.ToBuffer()
```

### Write a 3D scan

```js
import { E57WriterScan } from 'e57-js'

const scan = new E57WriterScan()
scan.GetHeader().guid = 'my-unique-scan-id'
scan.GetHeader().pointFields.cartesianXField = true
scan.GetHeader().pointFields.cartesianYField = true
scan.GetHeader().pointFields.cartesianZField = true

for (let i = 0; i < 100; i++) {
    const pt = new E57.LibE57.Point()
    pt.cartesianX = i * 0.1
    pt.cartesianY = 0
    pt.cartesianZ = 0
    scan.AddPoint(pt)
}

const scanIndex = await writer.AddScan(scan)
scan.Destroy()
```

### Write a scan with colour

```js
const scan = new E57WriterScan()
scan.GetHeader().pointFields.cartesianXField = true
scan.GetHeader().pointFields.cartesianYField = true
scan.GetHeader().pointFields.cartesianZField = true
scan.GetHeader().pointFields.colorRedField   = true
scan.GetHeader().pointFields.colorGreenField = true
scan.GetHeader().pointFields.colorBlueField  = true
scan.GetHeader().colorLimits.colorRedMaximum   = 255
scan.GetHeader().colorLimits.colorGreenMaximum = 255
scan.GetHeader().colorLimits.colorBlueMaximum  = 255

const pt = new E57.LibE57.Point()
pt.cartesianX = 1.0; pt.cartesianY = 2.0; pt.cartesianZ = 3.0
pt.colorRed = 255; pt.colorGreen = 0; pt.colorBlue = 0
scan.AddPoint(pt)
```

### Set the scanner position (pose)

If you know where the scanner was placed, you can store that as a pose. Use an identity quaternion `(w=1, x=0, y=0, z=0)` if there is no rotation:

```js
scan.GetHeader().pose.rotation.w = 1.0
scan.GetHeader().pose.rotation.x = 0.0
scan.GetHeader().pose.rotation.y = 0.0
scan.GetHeader().pose.rotation.z = 0.0
scan.GetHeader().pose.translation.x = 10.0   // scanner was 10 m east
scan.GetHeader().pose.translation.y = 20.0   // 20 m north
scan.GetHeader().pose.translation.z = 1.5    // 1.5 m above ground
```

### Write an image

```js
import { E57WriterImage } from 'e57-js'

// From a file path (Node.js)
const image = new E57WriterImage(
    'photo.jpg',
    E57.LibE57.Image2DType.ImageJPEG,
    E57.LibE57.Image2DProjection.ProjectionVisual
)
image.SetName('Front camera')

await writer.AddImage(image)
image.Destroy()
```

In the browser, use `E57WriterImage.FromBuffer` instead:

```js
// From a Uint8Array (browser or Node.js)
const image = E57WriterImage.FromBuffer(
    buffer,                                          // Uint8Array
    E57.LibE57.Image2DType.ImageJPEG,
    E57.LibE57.Image2DProjection.ProjectionVisual
)
image.SetName('Front camera')

await writer.AddImage(image)
image.Destroy()
```

### Close the file

Always call `Close()` when you are done. This finalises the file — skipping it will produce a corrupt file.

```js
writer.Close()  // returns a Uint8Array only when the writer was created with E57Writer.ToBuffer()
```

---

## Point fields reference

| Field | Description |
|---|---|
| `cartesianX` / `Y` / `Z` | 3D position in metres |
| `colorRed` / `Green` / `Blue` | RGB colour (0–255 or 0–65535) |
| `intensity` | Reflected signal strength |
| `sphericalRange` / `Azimuth` / `Elevation` | Position in spherical coordinates |
| `normalX` / `Y` / `Z` | Surface normal vector |
| `timeStamp` | When the point was captured |
| `rowIndex` / `columnIndex` | Grid position (structured scans) |
| `returnIndex` / `returnCount` | Multi-return laser data |

The fields a scan actually contains depend on how it was written. Check `scan.GetHeader().pointFields` to see which ones are present.

---

For more examples of how to read and write E57 files with this library, have a look at the test files:

- [tests/io.test.js](tests/io.test.js) — synchronous read/write examples
- [tests/io.async.test.js](tests/io.async.test.js) — async read/write examples

---

## License

MIT License

# e57-js

Node.js library for reading and writing **E57 point cloud files**.

> **What is a point cloud?** A point cloud is a collection of 3D points in space, typically captured by a laser scanner (LiDAR). Each point has X, Y, Z coordinates and optionally colour, intensity, and other data. E57 is the standard file format for storing them.

---

## Installation

```sh
npm install e57-js
```

Requires **Node.js 18 or higher**.

---

## Setup

Before doing anything, call `E57.Init()` once and wait for it to finish. This loads the underlying WebAssembly module.

```js
import { E57, E57Reader, E57Writer } from 'e57-js'

await E57.Init()

// Now you can create readers and writers
```

---

## Reading a file

### Open the file

```js
const reader = new E57Reader('scan.e57')

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
```

### Read in chunks (better for large files)

Reading millions of points all at once can use a lot of memory. Use `ScanPoints` to process them in smaller batches:

```js
await scan.ScanPoints(1000, (chunk) => {
    for (let i = 0; i < chunk.size(); i++) {
        const pt = chunk.get(i)
        // process pt...
    }
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

// Or save it directly to disk (extension is auto-detected)
await image.Save('output')   // writes output.jpeg or output.png
```

### Synchronous versions

Every async method has a sync equivalent if you prefer blocking calls:

```js
const points = scan.ReadScanSync()
const bytes  = image.ReadImageSync()
```

---

## Writing a file

### Create the file

```js
const writer = new E57Writer('output.e57')
```

### Write a 3D scan

First, describe what fields your points will have (the header), then pass your array of points:

```js
// 1 — describe the scan
const header = new E57.LibE57.Data3D()
header.guid = 'my-unique-scan-id'
header.pointFields.cartesianXField = true
header.pointFields.cartesianYField = true
header.pointFields.cartesianZField = true

// 2 — build the points
const points = []
for (let i = 0; i < 100; i++) {
    const pt = new E57.LibE57.Point()
    pt.cartesianX = i * 0.1
    pt.cartesianY = 0
    pt.cartesianZ = 0
    points.push(pt)
}

// 3 — write (async)
const scanIndex = await writer.AddScan(header, points)
```

### Write a scan with colour

```js
const header = new E57.LibE57.Data3D()
header.pointFields.cartesianXField = true
header.pointFields.cartesianYField = true
header.pointFields.cartesianZField = true
header.pointFields.colorRedField   = true
header.pointFields.colorGreenField = true
header.pointFields.colorBlueField  = true
header.colorLimits.colorRedMaximum   = 255
header.colorLimits.colorGreenMaximum = 255
header.colorLimits.colorBlueMaximum  = 255

const pt = new E57.LibE57.Point()
pt.cartesianX = 1.0; pt.cartesianY = 2.0; pt.cartesianZ = 3.0
pt.colorRed = 255; pt.colorGreen = 0; pt.colorBlue = 0  // red point
```

### Set the scanner position (pose)

If you know where the scanner was placed, you can store that as a pose. Use an identity quaternion `(w=1, x=0, y=0, z=0)` if there is no rotation:

```js
header.pose.rotation.w = 1.0
header.pose.rotation.x = 0.0
header.pose.rotation.y = 0.0
header.pose.rotation.z = 0.0
header.pose.translation.x = 10.0   // scanner was 10 m east
header.pose.translation.y = 20.0   // 20 m north
header.pose.translation.z = 1.5    // 1.5 m above ground
```

### Write an image

```js
import { E57WriterImage } from 'e57-js'

const image = new E57WriterImage(
    'photo.jpg',
    E57.LibE57.Image2DType.ImageJPEG,
    E57.LibE57.Image2DProjection.ProjectionVisual
)
image.setName('Front camera')

await writer.AddImage(image)
```

### Close the file

Always call `Close()` when you are done. This finalises the file — skipping it will produce a corrupt file.

```js
writer.Close()
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

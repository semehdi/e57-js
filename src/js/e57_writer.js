import { E57 } from "./e57_init.js"

import path from "path"
import fs from "fs"
import sharp from "sharp"

/**
 * Holds the data and metadata for a single 2D image to be written to an E57 file.
 *
 * Pass an instance of this class to `E57Writer.AddImage()`.
 *
 * @example
 * const image = new E57WriterImage(
 *     "photo.jpg",
 *     E57.LibE57.Image2DType.ImageJPEG,
 *     E57.LibE57.Image2DProjection.ProjectionVisual
 * )
 * image.SetName("Front camera")
 * image.SetRotation(w, x, y, z)
 * await writer.AddImage(image)
 */
export class E57WriterImage
{
    /**
     * @param {string} imagePath       - Path to the source image file on disk.
     * @param {number} imageType       - `Image2DType` enum value (e.g. `E57.LibE57.Image2DType.ImageJPEG`).
     * @param {number} imageProjection - `Image2DProjection` enum value (e.g. `E57.LibE57.Image2DProjection.ProjectionVisual`).
     */
    constructor(imagePath, imageType, imageProjection)
    {
        this._imgPath = imagePath;
        this._imgType = imageType;
        this._imgProjection = imageProjection;
        this._imageHeader = new E57.LibE57.ImageHeader();
    }

    /**
     * Creates an `E57WriterImage` from a `Uint8Array` instead of a file path.
     * Use this in the browser or any context where the file system is unavailable.
     *
     * @param {Uint8Array} buffer          - Raw image bytes.
     * @param {number}     imageType       - `Image2DType` enum value.
     * @param {number}     imageProjection - `Image2DProjection` enum value.
     * @returns {E57WriterImage}
     */
    static FromBuffer(buffer, imageType, imageProjection)
    {
        const img = new E57WriterImage(null, imageType, imageProjection);
        img._buffer = buffer;
        img._width = null;
        img._height = null;
        return img;
    }

    /**
     * Returns the source image path.
     *
     * @returns {string}
     */
    GetPath()
    {
        return this._imgPath;
    }

    /**
     * @param {string} imgPath
     */
    SetPath(imgPath)
    {
        this._imgPath = imgPath;
    }

    /**
     * Returns the `Image2DType` enum value for this image.
     *
     * @returns {number}
     */
    GetType()
    {
        return this._imgType
    }

    /**
     * @param {number} imgType - `Image2DType` enum value.
     */
    SetType(imgType)
    {
        this._imgType = imgType;
    }

    /**
     * Returns the `Image2DProjection` enum value for this image.
     *
     * @returns {number}
     */
    GetProjection()
    {
        return this._imgProjection;
    }

    /**
     * @param {number} imgProjection - `Image2DProjection` enum value.
     */
    SetProjection(imgProjection)
    {
        this._imgProjection = imgProjection;
    }

    /**
     * Resolves image dimensions, using the environment-appropriate API.
     * In the browser, uses `createImageBitmap`; in Node.js, uses `sharp`.
     * The result is cached after the first call.
     *
     * @returns {Promise<{width: number, height: number}>}
     */
    _resolveMetadata()
    {
        if (this._width != null && this._height != null)
            return Promise.resolve({ width: this._width, height: this._height });

        if (this._metadataPromise) return this._metadataPromise;

        if (typeof window !== 'undefined') {
            this._metadataPromise = this.GetBuffer().then(buf => {
                const blob = new Blob([buf])
                return createImageBitmap(blob).then(bitmap => {
                    const meta = { width: bitmap.width, height: bitmap.height }
                    bitmap.close()
                    this._width  = meta.width
                    this._height = meta.height
                    return meta
                })
            })
        } else {
            this._metadataPromise = this.GetBuffer().then(buf => sharp(buf).metadata()).then(meta => {
                this._width  = meta.width
                this._height = meta.height
                return meta
            })
        }

        return this._metadataPromise;
    }

    /**
     * Returns image metadata. Uses `sharp` in Node.js and `createImageBitmap` in the browser.
     *
     * @returns {Promise<{width: number, height: number}>}
     */
    GetMetadata()
    {
        return this._resolveMetadata();
    }

    /**
     * Returns `[width, height]` of the source image.
     *
     * @returns {Promise<[number, number]>}
     */
    GetDimensions()
    {
        return this._resolveMetadata().then(meta => [meta.width, meta.height]);
    }

    /**
     * @returns {Promise<number>} Width in pixels.
     */
    GetWidth()
    {
        return this._resolveMetadata().then(meta => meta.width);
    }

    /**
     * @returns {Promise<number>} Height in pixels.
     */
    GetHeight()
    {
        return this._resolveMetadata().then(meta => meta.height);
    }

    /**
     * Returns the `RigidBodyTransform` pose stored in the image header.
     *
     * @returns {object}
     */
    GetPose()
    {
        return this._imageHeader.pose;
    }

    /**
     * @param {object} imgPose - `RigidBodyTransform` value.
     */
    SetPose(imgPose)
    {
        this._imageHeader.pose = imgPose;
    }

    /**
     * @returns {string}
     */
    GetName()
    {
        return this._imageHeader.name;
    }

    /**
     * @param {string} imgName
     */
    SetName(imgName)
    {
        this._imageHeader.name = imgName;
    }

    /**
     * @returns {string}
     */
    GetGuid()
    {
        return this._imageHeader.guid;
    }

    /**
     * @param {string} imgGuid
     */
    SetGuid(imgGuid)
    {
        this._imageHeader.guid = imgGuid;
    }

    /**
     * Returns the underlying `ImageHeader` that will be written to the E57 file.
     *
     * @returns {object} `ImageHeader` struct from libE57Format.
     */
    GetHeader()
    {
        return this._imageHeader;
    }

    /**
     * Replaces the underlying `ImageHeader`.
     *
     * @param {object} imgHeader - `ImageHeader` struct from libE57Format.
     */
    SetHeader(imgHeader)
    {
        this._imageHeader.delete();
        this._imageHeader = imgHeader;
    }

    /**
     * Reads the source image from disk and returns its bytes as a `Uint8Array`.
     *
     * @returns {Uint8Array}
     */
    GetBufferSync()
    {
        if (this._buffer) return this._buffer;
        return new Uint8Array(fs.readFileSync(this.GetPath()));
    }

    GetBuffer()
    {
        if (this._buffer) return Promise.resolve(this._buffer);
        return fs.promises.readFile(this.GetPath()).then(buf => new Uint8Array(buf));
    }

    /**
     * Sets the translation component of the image pose.
     *
     * @param {number} x
     * @param {number} y
     * @param {number} z
     */
    SetTranslation(x, y ,z)
    {
        this._imageHeader.pose.translation.x = x;
        this._imageHeader.pose.translation.y = y;
        this._imageHeader.pose.translation.z = z;
    }

    /**
     * Sets the rotation component of the image pose as a unit quaternion.
     *
     * @param {number} w
     * @param {number} x
     * @param {number} y
     * @param {number} z
     */
    SetRotation(w, x, y, z)
    {
        this._imageHeader.pose.rotation.w = w;
        this._imageHeader.pose.rotation.x = x;
        this._imageHeader.pose.rotation.y = y;
        this._imageHeader.pose.rotation.z = z;
    }

    /**
     * Frees the underlying Emscripten `ImageHeader` C++ object.
     * Call this once the image has been written and is no longer needed.
     */
    Destroy()
    {
        this._imageHeader.delete()
        this._imageHeader = null
        this._buffer = null
    }
}

/**
 * Creates a new E57 file and writes 3D scans and 2D images to it.
 *
 * Call `Close()` when done to flush and finalise the file.
 *
 * @example
 * await E57.Init()
 * const writer = new E57Writer("output.e57")
 * await writer.AddScan(header, points)
 * await writer.AddImage(image)
 * writer.Close()
 */
export class E57Writer
{
    /**
     * Creates (or overwrites) the E57 file at `filePath`.
     *
     * @param {string} filePath - Absolute or relative path for the output `.e57` file.
     */
    constructor(filePath, toBuffer = false)
    {
        const guid = crypto.randomUUID();
        this._bufferFileMemFSFilePath = "/" + guid + ".e57";
        const inputFilePath = toBuffer ? this._bufferFileMemFSFilePath : path.join(E57.RootDir, path.resolve(filePath));
        this.writer = new E57.LibE57.E57Writer(inputFilePath);
        this._toBuffer = toBuffer;
    }

    /**
     * Creates an `E57Writer` that writes entirely to the Emscripten in-memory
     * filesystem instead of a file on disk. Call `Close()` when done — it will
     * return the completed file as a `Uint8Array`.
     *
     * Useful in browser environments or any context where writing to disk is not
     * possible or desirable.
     *
     * @returns {E57Writer}
     *
     * @example
     * await E57.Init()
     * const writer = E57Writer.ToBuffer()
     * writer.AddScanSync(header, points)
     * const bytes = writer.Close() // Uint8Array containing the full E57 file
     */
    static ToBuffer()
    {
        return new E57Writer("", true);
    }

    /**
     * Reads the image buffer from disk and writes it to the E57 file asynchronously.
     * The write I/O runs on a background thread.
     *
     * @param {E57WriterImage} image - The image to write.
     * @returns {Promise<number>} Resolves with the number of bytes written.
     */
    AddImage(image)
    {
        return Promise.all([image.GetBuffer(), image.GetMetadata()])
            .then(([bufferData, meta]) => this.writer.AddImage(
                image.GetHeader(), image.GetType(), image.GetProjection(),
                0, bufferData, bufferData.length, meta.width, meta.height
            ).then(Number));
    }

    /**
     * Writes a 3D scan to the file synchronously. Blocks until complete.
     *
     * @param {object}   scanHeader - `Data3D` struct describing the scan.
     * @param {object[]} points     - Array of `Point` objects.
     * @returns {number} Zero-based index assigned to the new scan.
     */
    AddScanSync(scanHeader, points)
    {
        return Number(this.writer.AddScanSync(scanHeader, points));
    }

    /**
     * Writes a 3D scan to the file asynchronously.
     * Point extraction runs on the main thread; the file write runs on a background thread.
     *
     * @param {object}   scanHeader - `Data3D` struct describing the scan.
     * @param {object[]} points     - Array of `Point` objects.
     * @returns {Promise<number>} Resolves with the zero-based index assigned to the new scan.
     */
    AddScan(scanHeader, points)
    {
        return this.writer.AddScan(scanHeader, points).then(Number);
    }

    /**
     * Flushes all pending data, finalises the E57 structure, and closes the
     * underlying writer. Must be called after all scans and images have been added.
     *
     * When the writer was created with `E57Writer.ToBuffer()`, `Close()` reads the
     * completed file from the Emscripten in-memory filesystem and returns it as a
     * `Uint8Array` — no file is written to disk. In all other cases the return
     * value is `undefined`.
     *
     * @returns {Uint8Array|undefined} The raw E57 bytes when using `ToBuffer()`,
     *   otherwise `undefined`.
     *
     * @example
     * // file on disk
     * const writer = new E57Writer('output.e57')
     * writer.AddScanSync(header, points)
     * writer.Close()
     *
     * @example
     * // in-memory buffer
     * const writer = E57Writer.ToBuffer()
     * writer.AddScanSync(header, points)
     * const bytes = writer.Close() // Uint8Array
     */
    Close()
    {
        this.writer.Close();
        this.writer.delete()
        if (this._toBuffer) {
            const bytes = E57.LibE57.FS.readFile(this._bufferFileMemFSFilePath);
            E57.LibE57.FS.unlink(this._bufferFileMemFSFilePath);
            return bytes;
        }
    }
}

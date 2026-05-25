import { E57 } from "./e57_init.js"

import path from "path"
import fs from "fs"

/**
 * Represents a single 3D scan inside an E57 file.
 *
 * Obtained via `E57Reader.GetScan(scanIdx)` — do not construct directly.
 */
export class E57ReaderScan {
    /**
     * @param {object} e57Reader - The raw Emscripten `E57Reader` instance.
     * @param {number} scanIdx   - Zero-based index of this scan inside the file.
     */
    constructor(e57Reader, scanIdx)
    {
        this.e57Reader = e57Reader;
        this.scanIdx = scanIdx;
    }

    /**
     * Returns the `Data3D` header for this scan.
     *
     * @returns {object} `Data3D` struct from libE57Format.
     */
    GetHeader()
    {
        return this.e57Reader.GetData3DHeader(this.scanIdx);
    }

    /**
     * Reads all points in this scan.
     *
     * @returns {Promise<Point[]>} Resolves with the full array of `Point` objects.
     */
    ReadScan()
    {
        var scanHeader = this.GetHeader();
        var scanPtsCount = scanHeader.pointCount;
        return this.ReadPoints(scanPtsCount);
    }

    /**
     * Reads up to `ptsCount` points starting from the current read position.
     *
     * @param {number} ptsCount - Maximum number of points to read.
     * @returns {Promise<Point[]>} Resolves with an array of `Point` objects.
     */
    ReadPoints(ptsCount)
    {
        return new Promise((resolve, reject) => {
            resolve(this.e57Reader.ReadScan(this.scanIdx, ptsCount));
        });
    }

    /**
     * Iterates through the scan in fixed-size chunks, invoking `callback`
     * sequentially for each chunk. Useful for streaming large scans without
     * loading all points into memory at once.
     *
     * @param {number}   chunkSize - Number of points per chunk.
     * @param {function(Point[]): Promise<void>|void} callback - Called once per chunk with the resolved points.
     * @returns {Promise<void>}
     *
     * @example
     * await scan.ScanPoints(1000, (points) => {
     *     console.log(`Received ${points.length} points`);
     * });
     */
    ScanPoints(chunkSize, callback)
    {
        const scanHeader = this.GetHeader();
        const scanPtsCount = scanHeader.pointCount;
        const chunks = Math.ceil(scanPtsCount / chunkSize);

        let chain = Promise.resolve();
        for (let iChunk = 0; iChunk < chunks; iChunk++)
            chain = chain.then(() => this.ReadPoints(chunkSize).then(callback));
        return chain;
    }
}

/**
 * Represents a 2D image (camera image) embedded in an E57 file.
 *
 * Obtained via `E57Reader.GetImage(imageIdx)` — do not construct directly.
 */
export class E57ReaderImage
{
    /**
     * @param {object} e57Reader  - The raw Emscripten `E57Reader` instance.
     * @param {number} imageIdx   - Zero-based index of this image inside the file.
     */
    constructor(e57Reader, imageIdx)
    {
        this._e57Reader = e57Reader;
        this._imageIdx = imageIdx;
    }

    /**
     * Returns the `ImageHeader` for this image.
     *
     * @returns {object} `ImageHeader` struct from libE57Format.
     */
    GetHeader()
    {
        return this._e57Reader.GetImage2DHeader(this._imageIdx);
    }

    /**
     * Reads the raw image bytes.
     *
     * @returns {Promise<Uint8Array>} Resolves with the image data buffer.
     */
    ReadImage()
    {
        return new Promise((resolve, reject) => {
            resolve(this._e57Reader.ReadImage(this._imageIdx));
        });
    }

    /**
     * Reads the image and converts it to a Base64-encoded string.
     *
     * @returns {Promise<string>} Resolves with the Base64 string.
     */
    ToBase64()
    {
        return this.ReadImage().then((imgData) => {
            return Buffer.from(imgData).toString("base64");
        });
    }

    /**
     * Returns the file extension that matches this image's type.
     *
     * @returns {string} `".jpeg"`, `".png"`, or `".jpg"` as a fallback.
     */
    Extension()
    {
        const imageType = this.GetHeader().imageType;

        switch (imageType) {
            case E57.LibE57.Image2DType.ImageJPEG:
                return ".jpeg";
            case E57.LibE57.Image2DType.ImagePNG:
                return ".png";
            case E57.LibE57.Image2DType.ImageMaskPNG:
                return ".png";
            case E57.LibE57.Image2DType.E57_JPEG_IMAGE:
                return ".jpeg";
            case E57.LibE57.Image2DType.E57_PNG_IMAGE:
                return ".png";
            case E57.LibE57.Image2DType.E57_PNG_IMAGE_MASK:
                return ".png"
            default:
                return ".jpg";
        }
    }

    /**
     * Saves the image to disk. The file extension in `filePath` is replaced with
     * the correct extension for this image's type (see `Extension()`).
     *
     * @param {string} filePath - Destination path (extension will be overwritten).
     * @returns {void}
     */
    Save(filePath)
    {
        const outExtension = this.Extension();
        const newFilePath = filePath.replace(path.extname(filePath), outExtension);
        return this.ReadImage().then((imgData) => {
            return fs.writeFile(newFilePath, imgData, (err) => {
                if (err) throw err;
            });
        })
    }
}

/**
 * Opens an existing E57 file for reading.
 *
 * Provides access to all 3D scans (`E57ReaderScan`) and 2D images
 * (`E57ReaderImage`) stored in the file.
 *
 * @example
 * await E57.Init()
 * const reader = new E57Reader("scan.e57")
 * const scan = reader.GetScan(0)
 * const points = await scan.ReadScan()
 */
export class E57Reader {
    /**
     * Opens the E57 file at `filePath` and initialises scan and image
     * accessor objects for every entry in the file.
     *
     * @param {string} filePath - Absolute or relative path to the `.e57` file.
     */
    constructor(filePath)
    {
        const absInputPath = path.resolve(filePath);
        const inputFilePath = path.join(E57.RootDir, absInputPath);
        this.reader = new E57.LibE57.E57Reader(inputFilePath);

        var scansCount = this.GetData3DCount();
        this.scans = new Array(scansCount);
        for (var iScan = 0; iScan < scansCount; iScan++)
        {
            this.scans[iScan] = new E57ReaderScan(this.reader, iScan);
        }

        var imagesCount = this.GetImage2DCount();
        this.images = new Array(imagesCount);
        for (var iImage = 0; iImage < imagesCount; iImage++)
        {
            this.images[iImage] = new E57ReaderImage(this.reader, iImage);
        }
    }

    /**
     * Returns the file-level E57 header.
     *
     * @returns {object} `E57Root` struct from libE57Format.
     */
    GetHeader()
    {
        return this.reader.GetHeader();
    }

    /**
     * Returns the number of 3D scans in the file.
     *
     * @returns {number}
     */
    GetData3DCount()
    {
        return this.reader.GetData3DCount();
    }

    /**
     * Returns the number of 2D images in the file.
     *
     * @returns {number}
     */
    GetImage2DCount()
    {
        return this.reader.GetImage2DCount();
    }

    /**
     * Returns the scan accessor at `scanIdx`.
     *
     * @param {number} scanIdx - Zero-based scan index.
     * @returns {E57ReaderScan}
     */
    GetScan(scanIdx)
    {
        return this.scans[scanIdx];
    }

    /**
     * Returns the image accessor at `imageIdx`.
     *
     * @param {number} imageIdx - Zero-based image index.
     * @returns {E57ReaderImage}
     */
    GetImage(imageIdx)
    {
        return this.images[imageIdx];
    }
}

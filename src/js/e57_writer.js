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
 * image.setName("Front camera")
 * image.setRotation(w, x, y, z)
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
     * Returns the source image path.
     *
     * @returns {string}
     */
    getPath()
    {
        return this._imgPath;
    }

    /**
     * @param {string} imgPath
     */
    setPath(imgPath)
    {
        this._imgPath = imgPath;
    }

    /**
     * Returns the `Image2DType` enum value for this image.
     *
     * @returns {number}
     */
    getType()
    {
        return this._imgType
    }

    /**
     * @param {number} imgType - `Image2DType` enum value.
     */
    setType(imgType)
    {
        this._imgType = imgType;
    }

    /**
     * Returns the `Image2DProjection` enum value for this image.
     *
     * @returns {number}
     */
    getProjection()
    {
        return this._imgProjection;
    }

    /**
     * @param {number} imgProjection - `Image2DProjection` enum value.
     */
    setProjection(imgProjection)
    {
        this._imgProjection = imgProjection;
    }

    /**
     * Returns the raw sharp metadata for the source image.
     *
     * @returns {Promise<import('sharp').Metadata>}
     */
    getMetadata()
    {
        return sharp(this._imgPath).metadata();
    }

    /**
     * Returns `[width, height]` of the source image.
     *
     * @returns {Promise<[number, number]>}
     */
    getDimensions()
    {
        return sharp(this._imgPath).metadata().then((meta) => {
            return meta.width, meta.height;
        });
    }

    /**
     * @returns {Promise<number>} Width in pixels.
     */
    getWidth()
    {
        return sharp(this._imgPath).metadata().then((meta) => {
            return meta.width;
        });
    }

    /**
     * @returns {Promise<number>} Height in pixels.
     */
    getHeight()
    {
        return sharp(this._imgPath).metadata().then((meta) => {
            return meta.height;
        });
    }

    /**
     * Returns the `RigidBodyTransform` pose stored in the image header.
     *
     * @returns {object}
     */
    getPose()
    {
        return this._imageHeader.pose;
    }

    /**
     * @param {object} imgPose - `RigidBodyTransform` value.
     */
    setPose(imgPose)
    {
        this._imageHeader.pose = imgPose;
    }

    /**
     * @returns {string}
     */
    getName()
    {
        return this._imageHeader.name;
    }

    /**
     * @param {string} imgName
     */
    setName(imgName)
    {
        this._imageHeader.name = imgName;
    }

    /**
     * @returns {string}
     */
    getGuid()
    {
        return this._imageHeader.guid;
    }

    /**
     * @param {string} imgGuid
     */
    setGuid(imgGuid)
    {
        this._imageHeader.guid = imgGuid;
    }

    /**
     * Returns the underlying `ImageHeader` that will be written to the E57 file.
     *
     * @returns {object} `ImageHeader` struct from libE57Format.
     */
    getHeader()
    {
        return this._imageHeader;
    }

    /**
     * Replaces the underlying `ImageHeader`.
     *
     * @param {object} imgHeader - `ImageHeader` struct from libE57Format.
     */
    setHeader(imgHeader)
    {
        this._imageHeader = imgHeader;
    }

    /**
     * Reads the source image from disk and returns its bytes as a `Uint8Array`.
     *
     * @returns {Promise<Uint8Array>}
     */
    getBuffer()
    {
        return fs.promises.readFile(this.getPath()).then((buffer) => {
            var bufferData = new Uint8Array(buffer);
            return bufferData;
        });
    }

    /**
     * Sets the translation component of the image pose.
     *
     * @param {number} x
     * @param {number} y
     * @param {number} z
     */
    setTrasnlation(x, y ,z)
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
    setRotation(w, x, y, z)
    {
        this._imageHeader.pose.rotation.w = w;
        this._imageHeader.pose.rotation.x = x;
        this._imageHeader.pose.rotation.y = y;
        this._imageHeader.pose.rotation.z = z;
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
    constructor(filePath)
    {
        const absInputPath = path.resolve(filePath);
        const inputFilePath = path.join(E57.RootDir, absInputPath);
        this.writer = new E57.LibE57.E57Writer(inputFilePath);
    }

    /**
     * Reads the image buffer from disk and writes it to the E57 file.
     *
     * @param {E57WriterImage} image - The image to write.
     * @returns {Promise<void>}
     */
    AddImage(image)
    {
        return image.getBuffer().then((buffer) => {
            var bufferData = new Uint8Array(buffer);
            return sharp(buffer).metadata().then((meta) => {
                const imgWidth = meta.width;
                const imgHeight = meta.height;
                this.writer.AddImage(image.getHeader(), image.getType(), image.getProjection(), 0, bufferData, bufferData.length, imgWidth, imgHeight);
            })
        })
    }

    /**
     * Writes a 3D scan to the file.
     *
     * @param {object}   scanHeader - `Data3D` struct describing the scan (point fields, bounds, …).
     * @param {object[]} points     - Array of `Point` objects (created via `new E57.LibE57.Point()`).
     * @returns {Promise<number>} Resolves with the zero-based index assigned to the new scan.
     */
    AddScan(scanHeader, points)
    {
        return new Promise((resolve, reject) => {
            var scanIdx = this.writer.AddScan(scanHeader, points);
            if (scanIdx >= 0) {
                resolve(scanIdx);
            } else {
                reject(new Error("Failed to add the scan !"));
            }
        });
    }

    /**
     * Flushes and closes the file. Must be called after all scans and images
     * have been added.
     */
    Close()
    {
        this.writer.Close();
    }
}

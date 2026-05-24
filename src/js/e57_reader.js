import { E57 } from "./e57_init.js"

import path from "path"
import fs from "fs"

export class E57ReaderScan {
    constructor(e57Reader, scanIdx)
    {
        this.e57Reader = e57Reader;
        this.scanIdx = scanIdx;
    }

    GetHeader()
    {
        return this.e57Reader.GetData3DHeader(this.scanIdx);
    }

    ReadScan()
    {
        var scanHeader = this.GetHeader();
        var scanPtsCount = scanHeader.pointCount;
        return this.ReadPoints(scanPtsCount);
    }

    ReadPoints(ptsCount)
    {
        return new Promise((resolve, reject) => {
            resolve(this.e57Reader.ReadScan(this.scanIdx, ptsCount));
        });
    }

    ScanPoints(chunkSize, callback)
    {
        var scanHeader = this.GetHeader();
        var scanPtsCount = scanHeader.pointCount;
        var chunks = Math.ceil(scanPtsCount / chunkSize);
        for (var iChunk = 0; iChunk < chunks; iChunk++)
        {
            var readPromise = this.ReadPoints(chunkSize);
            callback(readPromise);
        }
    }
}

export class E57ReaderImage 
{
    constructor(e57Reader, imageIdx)
    {
        this._e57Reader = e57Reader;
        this._imageIdx = imageIdx;
    }

    GetHeader()
    {
        return this._e57Reader.GetImage2DHeader(this._imageIdx);
    }

    ReadImage()
    {
        var image2dHeader = this.GetHeader();
        return new Promise((resolve, reject) => {
            var imgData = this._e57Reader.ReadImage(this._imageIdx);
            resolve(imgData);
        });
    }

    ToBase64()
    {
        return new Promise((resolve, reject) => {
            this.ReadImage().then((imgData) => {
                Buffer.from(imgData).toString("base64")
            })
        })
    }

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

    Save(filePath)
    {
        const outExtension = this.Extension();
        const newFilePath = filePath.replace(path.extname(filePath), outExtension);
        this.ReadImage().then((imgData) => {
            return fs.writeFile(newFilePath, imgData, (err) => {
                if (err) throw err;
            });
        })
    }
}

export class E57Reader {
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

    GetHeader()
    {
        return this.reader.GetHeader();
    }

    GetData3DCount()
    {
        return this.reader.GetData3DCount();
    }

    GetImage2DCount()
    {
        return this.reader.GetImage2DCount();
    }

    GetScan(scanIdx)
    {
        return this.scans[scanIdx];
    }

    GetImage(imageIdx)
    {
        return this.images[imageIdx];
    }
}

import { E57 } from "./e57_init.js"

import path from "path"
import fs from "fs"
import sharp from "sharp"

export class E57WriterImage
{
    constructor(imagePath, imageType, imageProjection)
    {
        this._imgPath = imagePath;
        this._imgType = imageType;
        this._imgProjection = imageProjection;
        this._imageHeader = new E57.LibE57.ImageHeader();
    }

    getPath()
    {
        return this._imgPath;
    }

    setPath(imgPath)
    {
        this._imgPath = imgPath; 
    }

    getType()
    {
        return this._imgType
    }

    setType(imgType)
    {
        this._imgType = imgType;
    }

    getProjection()
    {
        return this._imgProjection;
    }

    setProjection(imgProjection)
    {
        this._imgProjection = imgProjection;
    }

    getMetadata()
    {
        return sharp(this._imgPath).metadata();
    }

    getDimensions()
    {
        return sharp(this._imgPath).metadata().then((meta) => {
            return meta.width, meta.height;
        });
    }

    getWidth()
    {
        return sharp(this._imgPath).metadata().then((meta) => {
            return meta.width;
        });
    }

    getHeight()
    {
        return sharp(this._imgPath).metadata().then((meta) => {
            return meta.height;
        });
    }

    getPose()
    {
        return this._imageHeader.pose;
    }

    setPose(imgPose)
    {
        this._imageHeader.pose = imgPose;
    }

    getName()
    {
        return this._imageHeader.name;
    }

    setName(imgName)
    {
        this._imageHeader.name = imgName;
    }

    getGuid()
    {
        return this._imageHeader.guid;
    }

    setGuid(imgGuid)
    {
        this._imageHeader.guid = imgGuid;
    }

    getHeader()
    {
        return this._imageHeader;
    }

    setHeader(imgHeader)
    {
        this._imageHeader = imgheader; 
    }

    getBuffer()
    {
        return fs.promises.readFile(this.getPath()).then((buffer) => {
            var bufferData = new Uint8Array(buffer);
            return bufferData;
        });
    }

    setTrasnlation(x, y ,z)
    {
        this._imageHeader.pose.translation.x = x;
        this._imageHeader.pose.translation.y = y;
        this._imageHeader.pose.translation.z = z;
    }

    setRotation(w, x, y, z)
    {
        this._imageHeader.pose.rotation.w = w;
        this._imageHeader.pose.rotation.x = x;
        this._imageHeader.pose.rotation.y = y;
        this._imageHeader.pose.rotation.z = z;
    }
}

export class E57Writer
{
    constructor(filePath)
    {
        const absInputPath = path.resolve(filePath);
        const inputFilePath = path.join(E57.RootDir, absInputPath);
        this.writer = new E57.LibE57.E57Writer(inputFilePath);
    }

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

    AddScan(scanHeader, points)
    {
        var wBytes = this.writer.AddScan(scanHeader, points);
    }

    Close()
    {
        this.writer.Close();
    }
}

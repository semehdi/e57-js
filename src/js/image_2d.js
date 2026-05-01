import { E57Init } from "./e57_init.js";
import path from "path"
import fs from "fs"

export class Image2D 
{
    constructor(imgData, imgHeader) 
    {
        this._imgData = imgData;
        this._imgHeader = imgHeader;  
    }

    ToBase64()
    {
        return new Promise((resolve, reject) => {
            resolve(Buffer.from(this._imgData).toString("base64"));
        })
    }

    GetHeader()
    {
        return this._imgHeader;
    }

    Extension()
    {
        const imageType = this._imgHeader.imageType;

        switch (imageType) {
            case E57Init.WasmModule.Image2DType.ImageJPEG:
                return ".jpeg";
            case E57Init.WasmModule.Image2DType.ImagePNG:
                return ".png";
            case E57Init.WasmModule.Image2DType.ImageMaskPNG:
                return ".png";
            case E57Init.WasmModule.Image2DType.E57_JPEG_IMAGE:
                return ".jpeg";
            case E57Init.WasmModule.Image2DType.E57_PNG_IMAGE:
                return ".png";
            case E57Init.WasmModule.Image2DType.E57_PNG_IMAGE_MASK:
                return ".png"
            default:
                return ".jpg";
        }
    }

    Buffer()
    {
        return this._imgData;
    }

    Save(filePath)
    {
        const outExtension = this.Extension();
        const newFilePath = filePath.replace(path.extname(filePath), outExtension);
        var self = this;

        return fs.writeFile(newFilePath, self._imgData, (err) => {
            if (err) throw err;
        });
    }
}
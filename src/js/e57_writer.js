import { E57Init } from "./e57_init.js"

import path from "path"
import fs from "fs"
import sharp from "sharp"

export class E57Writer
{
    constructor(filePath)
    {
        const absInputPath = path.resolve(filePath);
        const inputFilePath = path.join(E57Init.RootDir, absInputPath);
        this.writer = new E57Init.WasmModule.E57Writer(inputFilePath);
    }

    AddImage(header, imageType, projection, startPos, imagePath)
    {
        return fs.promises.readFile(imagePath).then((buffer) => {
            var bufferData = new Uint8Array(buffer);
            return sharp(buffer).metadata().then((meta) => {
                const imgWidth = meta.width;
                const imgHeight = meta.height;
                this.writer.AddImage(header, imageType, projection, startPos, bufferData, bufferData.length);
            })
        })
    }

    Close()
    {
        this.writer.Close();
    }
}

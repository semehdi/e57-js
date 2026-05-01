import { E57Reader, E57Writer, E57Init } from './dist/index.js'

await E57Init.Init();

var writer = new E57Writer("file.e57");

var imageProjection = E57Init.WasmModule.Image2DProjection.ProjectionVisual;
var imageType = E57Init.WasmModule.Image2DType.ImageJPEG;
var imageHeader = new E57Init.WasmModule.ImageHeader();
var imagePath = "tests/data/images/image_1.jpg";

writer.AddImage(imageHeader, imageType, imageProjection, 0, imagePath).then(() => {
    console.log("Done !");
    writer.Close();
})

import { E57Reader, E57Writer, E57Init, E57WriterImage } from './dist/index.js'

await E57Init.Init();

var writer = new E57Writer("file.e57");

var imageProjection = E57Init.WasmModule.Image2DProjection.ProjectionVisual;
var imageType = E57Init.WasmModule.Image2DType.ImageJPEG;
var imageHeader = new E57Init.WasmModule.ImageHeader();
var imagePath = "tests/data/images/image_1.jpg";

var imageInstance = new E57WriterImage(imagePath, imageType, imageProjection);
imageInstance.setName("Image1");
imageInstance.setRotation(0.9, 2.7, 2.3, 2.1);
imageInstance.setTrasnlation(4.5, 5.8, 8.9);

writer.AddImage(imageInstance).then(() => {
    console.log("Done !");
    writer.Close();
})

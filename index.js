import { E57Reader, E57Writer, E57Init, E57WriterImage } from './dist/index.js'

await E57Init.Init();

var writer = new E57Writer("file__.e57");

var imageProjection = E57Init.WasmModule.Image2DProjection.ProjectionVisual;
var imageType = E57Init.WasmModule.Image2DType.ImageJPEG;
var imageHeader = new E57Init.WasmModule.ImageHeader();
var imagePath = "tests/data/images/image_1.jpg";

var scanHeader = new E57Init.WasmModule.Data3D();
scanHeader.pointCount = 1;
scanHeader.pointFields.cartesianXField = true;
scanHeader.pointFields.cartesianYField = true;
scanHeader.pointFields.cartesianZField = true;

var pt = new E57Init.WasmModule.Point();
pt.cartesianX = 2.2;
pt.cartesianY = 2.2;
pt.cartesianZ = 2.2;
var points = new Array(1);
points[0] = pt;

var imageInstance = new E57WriterImage(imagePath, imageType, imageProjection);
imageInstance.setName("Image1");
imageInstance.setRotation(0.9, 2.7, 2.3, 2.1);

writer.AddScan(scanHeader, points);
writer.Close();
console.log("Done");

var r = new E57Reader("./file__.e57");
var _header = r.GetHeader();
console.log(_header);
r.GetScan(0).ReadScan().then((data) => {
    console.log("Data : ", data);
    console.log(data.get(45).cartesianX);
})

import { E57Reader, E57Writer, E57Init, E57WriterImage } from './dist/index.js'

await E57Init.Init();

var writer = new E57Writer("file__.e57");

var imageProjection = E57Init.WasmModule.Image2DProjection.ProjectionVisual;
var imageType = E57Init.WasmModule.Image2DType.ImageJPEG;
var imageHeader = new E57Init.WasmModule.ImageHeader();
var imagePath = "tests/data/images/image_1.jpg";

var header = new E57Init.WasmModule.Data3D();

header.guid = "Cartesian Points Min/Max Float Header GUID";
header.pointFields.cartesianXField = true;
header.pointFields.cartesianYField = true;
header.pointFields.cartesianZField = true;

header.pointFields.pointRangeNodeType = E57Init.WasmModule.NumericalNodeType.ScaledInteger;
header.pointFields.pointRangeScale = 0.1;
header.pointFields.timeStampField = true;
header.pointFields.timeNodeType = E57Init.WasmModule.NumericalNodeType.ScaledInteger;
header.pointFields.timeScale = 0.1;

var pt = new E57Init.WasmModule.Point();
var numPoints = 1024;
var points = new Array(numPoints);

for (var i = 0; i < numPoints; i++)
{
    var pt = new E57Init.WasmModule.Point();
    pt.cartesianX = i;
    pt.cartesianY = i;
    pt.cartesianZ = i;
    points[i] = pt;
}

var imageInstance = new E57WriterImage(imagePath, imageType, imageProjection);
imageInstance.setName("Image1");
imageInstance.setRotation(0.9, 2.7, 2.3, 2.1);

writer.AddScan(header, points);
writer.Close();
console.log("Done");

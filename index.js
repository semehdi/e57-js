import { E57Reader, E57Writer, E57, E57WriterImage } from './dist/index.js'

await E57.Init();

var writer = new E57Writer("file__.e57");

console.log(Object.keys(E57.LibE57.Image2DProjection));

var imageProjection = E57.LibE57.Image2DProjection.ProjectionVisual;
var imageType = E57.LibE57.Image2DType.ImageJPEG;
var imageHeader = new E57.LibE57.ImageHeader();
var imagePath = "tests/data/images/image_1.jpg";

var header = new E57.LibE57.Data3D();

header.guid = "Cartesian Points Min/Max Float Header GUID";
header.pointFields.cartesianXField = true;
header.pointFields.cartesianYField = true;
header.pointFields.cartesianZField = true;

header.pointFields.pointRangeNodeType = E57.LibE57.NumericalNodeType.ScaledInteger;
header.pointFields.pointRangeScale = 0.1;
header.pointFields.timeStampField = true;
header.pointFields.timeNodeType = E57.LibE57.NumericalNodeType.ScaledInteger;
header.pointFields.timeScale = 0.1;

var pt = new E57.LibE57.Point();
var numPoints = 1024;
var points = new Array(numPoints);

for (var i = 0; i < numPoints; i++)
{
    var pt = new E57.LibE57.Point();
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

var reader = new E57Reader("./Station018.e57");
var image = reader.GetImage(0);
image.Save("./image.jpg").then(() => {
    console.log("Image creation done !");
})

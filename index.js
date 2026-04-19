import e57 from './build/libe57-js.js';
import path from "path"

var e57b = await e57();

const rootDir = "/root";
e57b.FS.mkdir(rootDir);
e57b.FS.mount(e57b.FS.filesystems.NODEFS, {root : '/'}, rootDir);

const inputFile = path.join(rootDir, "/app/pump.e57");
var options = { checksumPolicy: e57b.ChecksumPolicy.ChecksumAll };
var reader = new e57b.E57(inputFile);
var header = reader.GetData3DHeader(0);
console.log("Data 3D count : " + reader.GetData3DCount());
console.log("Images 2D count : " + reader.GetImage2DCount());
const data = reader.ReadData(1);
var c = 0;
var f = 0;
for (let i = 0; i < data.size(); i++) {
  const cartesianX = data.get(i).cartesianX;
  const v = data.get(i).cartesianInvalidState;
  if (cartesianX != 0)
    c++;

  if (v == 0)
    f++;
}

console.log(c);
console.log(f);

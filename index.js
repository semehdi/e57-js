import { E57Reader, E57Init } from './dist/index.js'

await E57Init.Init();

var reader = new E57Reader("Station018.e57");
var scan = reader.GetScan(0);
var image = reader.GetImage(0);

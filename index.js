import { E57Reader } from './dist/index.js'

var eFile = new E57Reader();
await eFile.Open("Station018.e57");

console.log("Hello World");


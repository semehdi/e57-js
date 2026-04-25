import e57 from "../../build/libe57-js.js"
import path from "path"

export class E57Reader {
    constructor() {}

    Open(filePath)
    {
        var self = this;
        return e57().then(function(e57Module) {
            const rootDir = "/root";
            e57Module.FS.mkdir(rootDir);
            e57Module.FS.mount(e57Module.FS.filesystems.NODEFS, {root : '/'}, rootDir);
            const absInputPath = path.resolve(filePath);
            const inputFilePath = path.join(rootDir, absInputPath);
            const options = { checksumPolicy: e57Module.ChecksumPolicy.ChecksumAll };
            self.reader = new e57Module.E57(inputFilePath);
        })
    }
}

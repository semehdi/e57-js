import e57 from "../../build/libe57-js.js"

export class E57 {
    static LibE57 = null;
    static RootDir = "/root ";

    static Init()
    {
        return e57().then(function(e57Module) {
            e57Module.FS.mkdir(E57.RootDir);
            e57Module.FS.mount(e57Module.FS.filesystems.NODEFS, {root : '/'}, E57.RootDir);
            E57.LibE57 = e57Module;
        })
    }
}

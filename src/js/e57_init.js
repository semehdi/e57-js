import e57 from "../../build/libe57-js.js"

export class E57Init {
    static WasmModule = null;
    static RootDir = "/root ";

    static Init()
    {
        return e57().then(function(e57Module) {
            e57Module.FS.mkdir(E57Init.RootDir);
            e57Module.FS.mount(e57Module.FS.filesystems.NODEFS, {root : '/'}, E57Init.RootDir);
            E57Init.WasmModule = e57Module;
        })
    }
}
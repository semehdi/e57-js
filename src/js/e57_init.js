import e57 from "../../build/libe57-js.js"

/**
 * Static class that initialises the underlying Emscripten/WebAssembly module and
 * exposes the compiled libE57Format API.
 *
 * `E57.Init()` **must** be awaited before creating any `E57Reader` or `E57Writer`.
 *
 * @example
 * import { E57 } from './e57_init.js'
 * await E57.Init()
 * console.log(E57.LibE57) // the raw Emscripten module
 */
export class E57 {
    /**
     * The initialised Emscripten module. `null` until `Init()` resolves.
     *
     * Exposes the full libE57Format binding surface, including:
     * - Constructors: `E57Reader`, `E57Writer`, `Data3D`, `Point`, `ImageHeader`
     * - Enums: `Image2DType`, `Image2DProjection`, `NumericalNodeType`, `NodeType`, …
     * - Helper types: `Translation`, `Quaternion`, `RigidBodyTransform`, `CartesianBounds`, …
     *
     * @type {object|null}
     */
    static LibE57 = null;

    /**
     * Virtual root path under which the host filesystem is mounted inside the
     * Emscripten NODEFS. All file paths passed to `E57Reader` / `E57Writer`
     * constructors are automatically prefixed with this value.
     *
     * @type {string}
     */
    static RootDir = "/root ";

    /**
     * Loads the WebAssembly module, mounts the host filesystem via NODEFS, and
     * populates `E57.LibE57`. Safe to call multiple times — resolves immediately
     * if the module is already initialised.
     *
     * @returns {Promise<void>} Resolves when the module is ready to use.
     *
     * @example
     * await E57.Init()
     */
    static Init()
    {
        const isBrowser = typeof window !== 'undefined';
        
        if (E57.LibE57 !== null)
            return Promise.resolve();

        return e57().then(function(e57Module) {
            e57Module.FS.mkdir(E57.RootDir);
            e57Module.FS.mount(e57Module.FS.filesystems.NODEFS, {root : '/'}, E57.RootDir);
            E57.LibE57 = e57Module;
        });
    }
}

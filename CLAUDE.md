# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What This Project Is

`e57-js` is a Node.js library for reading and writing E57 point cloud files. It works by compiling C++ code (built on top of [libE57Format](https://github.com/asmaloney/libE57Format)) to a WebAssembly module via Emscripten, then wrapping it in a JavaScript API.

## Build

### Prerequisites

The C++ → WASM compilation requires Emscripten. The Dockerfile sets up the full build environment:

```sh
docker build -t e57-js .
docker run -it -v $(pwd):/app e57-js
```

### Compiling the WebAssembly module (inside Docker or with emscripten installed)

```sh
mkdir build && cd build
emcmake cmake ..
emmake make
```

This produces `build/libe57-js.js` and `build/libe57-js.wasm`.

### Bundling the JavaScript layer

```sh
npx tsup
```

This bundles `src/js/` into `dist/index.js` and runs `scripts/post_build.js`, which copies the WASM artifacts from `build/` into `dist/`.

The final distributable is `dist/` — it contains `index.js`, `libe57-js.js`, and `libe57-js.wasm`.

### Running the example

```sh
node index.js
```

## Architecture

```
src/cpp/          C++ source compiled to WASM via Emscripten
  main.cpp        All Emscripten bindings (EMSCRIPTEN_BINDINGS block)
  e57_reader.cpp  C++ E57 reader logic
  e57_writer.cpp  C++ E57 writer logic
  point.cpp       Point struct
  image_header.cpp  ImageHeader struct

include/          C++ headers for the above

libE57Format/     Upstream libE57Format library (built as a static library)

src/js/           JavaScript layer (bundled by tsup → dist/)
  e57_init.js     Loads the WASM module, initializes Emscripten NODEFS
  e57_reader.js   JS wrappers: E57Reader, E57ReaderScan, E57ReaderImage
  e57_writer.js   JS wrappers: E57Writer, E57WriterImage
  index.js        Re-exports all JS classes

build/            CMake output (WASM artifacts land here)
dist/             Final bundle consumed by users (JS + WASM)
```

### Key architectural detail: filesystem path mapping

The Emscripten module uses NODEFS to mount the real host filesystem at the path `E57.RootDir` (`"/root "`—note the trailing space). All file paths passed to the C++ layer are prefixed with this virtual root. The JS wrappers in `e57_reader.js` and `e57_writer.js` handle this automatically using `path.resolve` + `path.join(E57.RootDir, absPath)`.

### Initialization requirement

`E57.Init()` must be called and awaited before creating any `E57Reader` or `E57Writer`. It loads the WASM module and mounts the filesystem.

### C++/JS boundary

`src/cpp/main.cpp` contains the entire `EMSCRIPTEN_BINDINGS` block — every C++ class, enum, and function exposed to JS is declared there. When adding new C++ functionality, both the implementation file and a binding entry in `main.cpp` are needed.

The C++ `E57Reader` and `E57Writer` classes (in `include/`) are thin wrappers around `libE57Format`'s `SimpleReader`/`SimpleWriter` API. The JS `E57Reader` and `E57Writer` classes (in `src/js/`) wrap those WASM-bound classes.

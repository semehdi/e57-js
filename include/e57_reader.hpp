#pragma once

#include <iostream>
#include <unordered_map>
#include <vector>
#include <thread>
#include <emscripten.h>
#include <emscripten/val.h>
#include <emscripten/threading.h>
#include <cstdlib>
#include <pthread.h>

#include "E57SimpleReader.h"
#include "E57SimpleData.h"
#include "point.hpp"
#include "image_header.hpp"
#include "em_promise.hpp"

using namespace e57;

/**
 * @brief Thin Emscripten-friendly wrapper around libE57Format's SimpleReader.
 *
 * Exposes synchronous metadata accessors and asynchronous point/image readers
 * that run I/O on a background pthread and resolve a JS Promise on the main
 * runtime thread.
 *
 * All async methods return an `emscripten::val` that is a JS Promise. They
 * must be called from the main thread; the actual I/O happens on a detached
 * worker thread.
 */
class E57Reader
{
public:
    /**
     * @brief Opens an E57 file for reading.
     * @param filePath Absolute path inside the Emscripten virtual filesystem
     *                 (i.e. prefixed with E57.RootDir by the JS wrapper).
     * @throws e57::E57Exception if the file cannot be opened or parsed.
     */
    E57Reader(const std::string& filePath);

    /**
     * @brief Returns the file-level E57 header.
     * @return Populated `E57Root` struct.
     */
    E57Root GetHeader();

    /**
     * @brief Returns the number of 3D scans in the file.
     */
    int64_t GetData3DCount();

    /**
     * @brief Returns the number of 2D images in the file.
     */
    int64_t GetImage2DCount();

    /**
     * @brief Returns the `Data3D` header for a single scan.
     * @param dataIdx Zero-based scan index.
     * @return Populated `Data3D` struct.
     */
    Data3D GetData3DHeader(int64_t dataIdx);

    /**
     * @brief Returns the combined header and size information for a single 2D image.
     * @param imageIdx Zero-based image index.
     * @return Populated `ImageHeader` struct.
     * @throws std::runtime_error if size information cannot be retrieved.
     */
    ImageHeader GetImage2DHeader(int64_t imageIdx);

    /**
     * @brief Reads up to `ptsSize` points from the scan at `scanIdx` asynchronously.
     *
     * Reading is stateful: successive calls advance an internal cursor so that
     * large scans can be streamed in fixed-size chunks. Call `ResetScanReader`
     * to start over from the beginning.
     *
     * I/O runs on a detached pthread. The Promise resolves on the main runtime
     * thread with a `VectorPoint` (Emscripten-bound `std::vector<Point>`).
     *
     * @param scanIdx  Zero-based scan index.
     * @param ptsSize  Maximum number of points to read in this call.
     * @return JS Promise that resolves with a `VectorPoint`, or rejects with
     *         an error string if `ptsSize < 0`.
     */
    emscripten::val ReadScan(int64_t scanIdx, int64_t ptsSize);

    /**
     * @brief Reads up to `ptsSize` points from the scan at `scanIdx` synchronously.
     *
     * Stateful in the same way as `ReadScan` — successive calls advance the
     * internal cursor. Blocks the calling thread until the read is complete.
     *
     * @param scanIdx  Zero-based scan index.
     * @param ptsSize  Maximum number of points to read.
     * @return `VectorPoint` (Emscripten-bound `std::vector<Point>`).
     * @throws std::runtime_error if `ptsSize < 0`.
     */
    emscripten::val ReadScanSync(int64_t scanIdx, int64_t ptsSize);

    /**
     * @brief Reads the raw image bytes for the image at `imageIdx` asynchronously.
     *
     * I/O runs on a detached pthread. The Promise resolves on the main runtime
     * thread with a zero-copy `Uint8Array` that views WASM's SharedArrayBuffer
     * directly. The underlying buffer is freed automatically via a JS
     * `FinalizationRegistry` when the `Uint8Array` is garbage-collected.
     *
     * @param imageIdx Zero-based image index.
     * @return JS Promise that resolves with a `Uint8Array` of the raw image
     *         bytes, or rejects with an error string.
     */
    emscripten::val ReadImage(int64_t imageIdx);

    /**
     * @brief Reads the raw image bytes for the image at `imageIdx` synchronously.
     *
     * Blocks the calling thread until the image is fully read. Returns a
     * zero-copy `Uint8Array` identical to what the async version resolves with.
     *
     * @param imageIdx Zero-based image index.
     * @return `Uint8Array` of the raw image bytes.
     * @throws std::runtime_error if the image cannot be read.
     */
    emscripten::val ReadImageSync(int64_t imageIdx);

    /**
     * @brief Resets the internal streaming cursor for a scan back to the start.
     *
     * After this call the next `ReadScan` for `scanIdx` will read from the
     * beginning of the scan again.
     *
     * @param scanIdx Zero-based scan index.
     */
    void ResetScanReader(int64_t scanIdx);

    /**
     * @brief Sanity-check Promise used during development.
     *
     * Spawns a thread that immediately posts back to the main thread and
     * resolves the Promise with the integer `42`.
     *
     * @return JS Promise that resolves with `42`.
     */
    emscripten::val TestPromise();

    ~E57Reader();

private:
    Reader* mReader;
    std::unordered_map<int64_t, std::shared_ptr<CompressedVectorReader>> mScanReaders;
    std::unordered_map<int64_t, Data3DPointsDouble*> mScanDataPoints;
    std::unordered_map<int64_t, int64_t> mReadPtsCount;

    /** Performs the actual image I/O for `imageIdx`, filling `eps` with the result. */
    void FetchImage(int64_t imageIdx, ImagePromiseSig<uint8_t>& eps);

    /** Performs the actual scan I/O for `scanIdx`, filling `eps` with up to `ptsSize` points. */
    void FetchScan(int64_t scanIdx, int64_t ptsSize, ScanPromiseSig<Point>& eps);

    /** Allocates a `CompressedVectorReader` and its data buffer for `scanIdx`. */
    void MakeScanReader(int64_t scanIdx, int64_t chunkSize);

    /** Releases the reader and data buffer for `scanIdx`. */
    void DestroyScanReader(int64_t scanIdx);

    /** Returns true if a `CompressedVectorReader` is currently open for `scanIdx`. */
    bool IsReaderValid(int64_t scanIdx);
};

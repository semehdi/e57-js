#pragma once

#include <string>
#include <thread>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <emscripten/threading.h>

#include "image_header.hpp"
#include "E57SimpleWriter.h"
#include "E57SimpleData.h"
#include "em_promise.hpp"
#include "point.hpp"

using namespace e57;

struct WriteImageSig
{
    EmPromise*           promise;
    Image2D              imgHeader;
    Image2DType          imageType;
    Image2DProjection    imageProjection;
    int64_t              startPos;
    std::vector<uint8_t> data;
    int64_t              byteCount;
    int64_t              result  = 0;
    std::string          error;
    bool                 success = false;
};

struct WriteScanSig
{
    EmPromise*          promise;
    Data3D              header;
    Data3DPointsDouble* pointsData = nullptr;
    int64_t             result     = 0;
    std::string         error;
    bool                success    = false;
};

/**
 * @brief Thin Emscripten-friendly wrapper around libE57Format's SimpleWriter.
 *
 * Exposes synchronous and asynchronous methods for writing 3D scans and 2D
 * images to an E57 file. The async variants off-load the file I/O to a
 * background pthread and resolve a JS Promise on the main runtime thread.
 *
 * Call `Close()` after all data has been written to flush and finalise the file.
 */
class E57Writer
{
public:
    /**
     * @brief Creates (or overwrites) an E57 file at `filePath`.
     * @param filePath Absolute path inside the Emscripten virtual filesystem
     *                 (i.e. prefixed with E57.RootDir by the JS wrapper).
     * @throws e57::E57Exception if the file cannot be created.
     */
    E57Writer(const std::string& filePath);

    /**
     * @brief Writes a 2D image to the file synchronously. Blocks until complete.
     * @param image2DHeader Populated `ImageHeader` describing the image metadata.
     * @param imageType     Format of the image data (JPEG, PNG, etc.).
     * @param imageProjection Projection type (visual, spherical, pinhole, cylindrical).
     * @param startPos      Byte offset within the image blob to start writing from.
     * @param jsArray       JS typed array containing the raw image bytes.
     * @param byteCount     Number of bytes to write from `jsArray`.
     * @param width         Image width in pixels.
     * @param height        Image height in pixels.
     * @return Number of bytes written.
     */
    int64_t AddImageSync(
        ImageHeader image2DHeader,
        Image2DType imageType,
        Image2DProjection imageProjection,
        int64_t startPos,
        const emscripten::val& jsArray,
        int64_t byteCount,
        int32_t width,
        int32_t height
    );

    /**
     * @brief Writes a 2D image to the file asynchronously.
     *
     * The JS array is extracted on the main thread (required for `emscripten::val`
     * access), then the actual file write runs on a detached pthread.
     *
     * @param image2DHeader Populated `ImageHeader` describing the image metadata.
     * @param imageType     Format of the image data (JPEG, PNG, etc.).
     * @param imageProjection Projection type (visual, spherical, pinhole, cylindrical).
     * @param startPos      Byte offset within the image blob to start writing from.
     * @param jsArray       JS typed array containing the raw image bytes.
     * @param byteCount     Number of bytes to write from `jsArray`.
     * @param width         Image width in pixels.
     * @param height        Image height in pixels.
     * @return JS Promise that resolves with the number of bytes written,
     *         or rejects with an error string.
     */
    emscripten::val AddImage(
        ImageHeader image2DHeader,
        Image2DType imageType,
        Image2DProjection imageProjection,
        int64_t startPos,
        const emscripten::val& jsArray,
        int64_t byteCount,
        int32_t width,
        int32_t height
    );

    /**
     * @brief Writes a 3D scan to the file synchronously. Blocks until complete.
     * @param scanHeader `Data3D` struct describing the scan (point fields, bounds, etc.).
     * @param jsArray    JS array of bound `Point` objects to write.
     * @return Zero-based index assigned to the new scan in the file.
     */
    int64_t AddScanSync(
        Data3D scanHeader,
        const emscripten::val& jsArray
    );

    /**
     * @brief Writes a 3D scan to the file asynchronously.
     *
     * Point extraction from the JS array runs on the main thread (required for
     * `emscripten::val` access), then the file write runs on a detached pthread.
     *
     * @param scanHeader `Data3D` struct describing the scan (point fields, bounds, etc.).
     * @param jsArray    JS array of bound `Point` objects to write.
     * @return JS Promise that resolves with the zero-based scan index,
     *         or rejects with an error string.
     */
    emscripten::val AddScan(
        Data3D scanHeader,
        const emscripten::val& jsArray
    );

    /**
     * @brief Flushes and closes the E57 file.
     *
     * Must be called after all scans and images have been added. Safe to call
     * multiple times — subsequent calls are no-ops.
     */
    void Close();

    ~E57Writer();

private:
    /** Underlying libE57Format simple writer. Owns the open file handle. */
    Writer* mWriter;

    /**
     * Core image write shared by `AddImageSync` and `AddImage`.
     * @param sig Input/output: contains the prepared header and data buffer;
     *            filled with the byte count on success or an error string on failure.
     */
    void mAddImage(WriteImageSig& sig);

    /**
     * Core scan write shared by `AddScanSync` and `AddScan`.
     * @param sig Input/output: contains the prepared header and points data;
     *            filled with the scan index on success or an error string on failure.
     */
    void mAddScan(WriteScanSig& sig);
};

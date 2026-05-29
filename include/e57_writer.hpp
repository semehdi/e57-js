#pragma once

#include <string>
#include <thread>

#include "image_header.hpp"
#include "E57SimpleWriter.h"
#include "E57SimpleData.h"
#include "em_promise.hpp"
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <emscripten/threading.h>
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

class E57Writer
{
public:
    E57Writer(const std::string& filePath);

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

    int64_t AddScanSync(
        Data3D scanHeader,
        const emscripten::val& jsArray
    );

    emscripten::val AddScan(
        Data3D scanHeader,
        const emscripten::val& jsArray
    );

    void Close();

    ~E57Writer();

private:
    Writer* mWriter;

    void FetchAddImage(WriteImageSig& sig);
    void FetchAddScan(WriteScanSig& sig);
};

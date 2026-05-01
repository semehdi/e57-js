#pragma once

#include <string>

#include "image_header.hpp"
#include "E57SimpleWriter.h"
#include "E57SimpleData.h"
#include <emscripten/bind.h>
#include <emscripten/val.h>

using namespace e57;

class E57Writer
{
public:
    E57Writer(const std::string& filePath);

    int64_t AddImage(
        ImageHeader image2DHeader, 
        Image2DType imageType, 
        Image2DProjection imageProjection, 
        int64_t startPos, 
        const emscripten::val& jsArray, 
        int64_t byteCount
    );

    void Close();

    ~E57Writer();

private:
    Writer* mWriter;
};

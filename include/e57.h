#pragma once

#include <iostream>
#include <unordered_map>
#include <vector>
#include <emscripten/val.h>

#include "E57SimpleReader.h"
#include "E57SimpleData.h"
#include "point.h"
#include "image_header.h"

using namespace e57;

class E57
{
public:
    E57(const std::string& filePath);

    E57Root GetHeader();
    int64_t GetData3DCount();
    int64_t GetImage2DCount();
    Data3D GetData3DHeader(int64_t dataIdx);
    ImageHeader GetImage2DHeader(int64_t imageIdx);
    std::vector<Point> ReadScan(int64_t scanIdx);
    emscripten::val ReadImage(int64_t imageIdx);

    ~E57();

private:
    Reader* mReader;
};
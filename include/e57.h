#pragma once

#include <iostream>

#include "E57SimpleReader.h"
#include "E57SimpleData.h"

using namespace e57;

class E57
{
public:
    E57(const std::string& filePath);

    E57Root GetHeader();
    int64_t GetData3DCount();
    int64_t GetImage2DCount();
    Data3D GetData3DHeader(int64_t dataIdx);

    ~E57();

private:
    Reader* mReader;
};
#pragma once

#include <iostream>
#include <unordered_map>
#include <vector>
#include <thread>
#include <emscripten.h>
#include <emscripten/val.h>

#include "E57SimpleReader.h"
#include "E57SimpleData.h"
#include "point.hpp"
#include "image_header.hpp"

using namespace e57;

class E57Reader
{
public:
    E57Reader(const std::string& filePath);

    E57Root GetHeader();
    int64_t GetData3DCount();
    int64_t GetImage2DCount();
    Data3D GetData3DHeader(int64_t dataIdx);
    ImageHeader GetImage2DHeader(int64_t imageIdx);
    std::vector<Point> ReadScan(int64_t scanIdx, int64_t ptsSize);
    emscripten::val ReadImage(int64_t imageIdx);
    void ResetScanReader(int64_t scanIdx);
 
    ~E57Reader();

private:
    Reader* mReader;
    std::unordered_map<int64_t, std::shared_ptr<CompressedVectorReader>> mScanReaders;
    std::unordered_map<int64_t, Data3DPointsDouble*> mScanDataPoints;
    std::unordered_map<int64_t, int64_t> mReadPtsCount;

    void MakeScanReader(int64_t scanIdx, int64_t chunkSize);
    void DestroyScanReader(int64_t scanIdx);
    bool IsReaderValid(int64_t scanIdx);
};
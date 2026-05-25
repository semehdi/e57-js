#include "e57_reader.hpp"
#include "image_header.hpp"

#include <atomic>
#include <pthread.h>
#include <emscripten/threading.h>
#include <emscripten/promise.h>

E57Reader::E57Reader(const std::string& filePath) {
    this->mReader = new Reader(filePath);
}

E57Root E57Reader::GetHeader()
{
    E57Root e57Root;
    this->mReader->GetE57Root(e57Root);
    return e57Root;
}

Data3D E57Reader::GetData3DHeader(int64_t dataIdx)
{
    Data3D data3DHeader;
    this->mReader->ReadData3D(dataIdx, data3DHeader);
    return data3DHeader;
}

ImageHeader E57Reader::GetImage2DHeader(int64_t imageIdx)
{
    Image2D image2d;
    this->mReader->ReadImage2D(imageIdx, image2d);
    Image2DProjection imageProjection;
    Image2DType imageType;
    int64_t imageWidth, imageHeight, imageSize;
    Image2DType imageMaskType, imageVisualType;
    
    const bool isSucess = this->mReader->GetImage2DSizes(imageIdx, imageProjection, imageType, 
        imageWidth, imageHeight, imageSize, imageMaskType, imageVisualType);
    if (!isSucess)
        throw std::runtime_error("Cannot get the header of the image !");

    ImageHeader imageHeader = ImageHeader(image2d);
    imageHeader.imageProjection = imageProjection;
    imageHeader.imageType = imageType;
    imageHeader.width = imageWidth;
    imageHeader.height = imageHeight;
    imageHeader.imageMaskType = imageMaskType;
    imageHeader.imageVisualType = imageVisualType;
    imageHeader.imageSize = imageSize;
    return imageHeader;
}

int64_t E57Reader::GetData3DCount()
{
    return this->mReader->GetData3DCount();
}

int64_t E57Reader::GetImage2DCount()
{
    return this->mReader->GetImage2DCount();
}

std::vector<Point> E57Reader::ReadScan(int64_t scanIdx, int64_t ptsSize)
{
    std::vector<Point> pts = std::vector<Point>();

    if (ptsSize < 0) return pts;

    const Data3D scanHeader = this->GetData3DHeader(scanIdx);
    const int64_t scanPtsCount = scanHeader.pointCount;
    
    if (this->mReadPtsCount[scanIdx] >= scanPtsCount || this->mReadPtsCount[scanIdx] < 0)
    {
        this->ResetScanReader(scanIdx);
        return pts;
    }

    ptsSize = std::min(ptsSize, scanPtsCount);
    ptsSize = std::min(scanPtsCount - this->mReadPtsCount[scanIdx], ptsSize);

    if (!this->IsReaderValid(scanIdx)) 
        this->MakeScanReader(scanIdx, ptsSize);
    
    Data3DPointsDouble* pointsData = this->mScanDataPoints[scanIdx];
    std::shared_ptr<CompressedVectorReader> dataReader = this->mScanReaders[scanIdx];

    pts.resize(ptsSize);

    int64_t count = 0;
    unsigned size = 0;

    while (size = dataReader->read())
    {
        for (long i = 0; i < size; i++)
        {
            const Point pt = Point(pointsData, i);
            pts[count++] = pt;
            this->mReadPtsCount[scanIdx]++;
            if (count >= ptsSize) break;
        }
        if (count >= ptsSize) break;
    }
    return pts;
}

emscripten::val E57Reader::ReadImage(int64_t imageIdx)
{
    Image2DProjection imageProjection;
    Image2DType       imageType, imageMaskType, imageVisualType;
    int64_t           imageWidth, imageHeight, imageSize;

    if (!this->mReader->GetImage2DSizes(imageIdx, imageProjection, imageType,
            imageWidth, imageHeight, imageSize, imageMaskType, imageVisualType))
        throw std::runtime_error("Cannot read image: GetImage2DSizes failed");

    std::vector<uint8_t> bytes(imageSize);
    const size_t rBytes = this->mReader->ReadImage2DData(
        imageIdx, imageProjection, imageType, bytes.data(), 0, imageSize);

    if (static_cast<int64_t>(rBytes) != imageSize)
        throw std::runtime_error("Cannot read image: incomplete read");

    // Copy bytes into a JS-owned Uint8Array before the vector goes out of scope.
    return emscripten::val::global("Uint8Array").new_(
        emscripten::val(emscripten::typed_memory_view(bytes.size(), bytes.data()))
    );
}

void E57Reader::MakeScanReader(int64_t scanIdx, int64_t chunkSize)
{
    Data3D scanHeader = this->GetData3DHeader(scanIdx);
    this->mScanDataPoints[scanIdx] = new Data3DPointsDouble(scanHeader);
    this->mScanReaders[scanIdx] = this->mReader->SetUpData3DPointsData(scanIdx, chunkSize, *this->mScanDataPoints[scanIdx]);
}

void E57Reader::ResetScanReader(int64_t scanIdx)
{
    this->DestroyScanReader(scanIdx);
    this->mReadPtsCount[scanIdx] = 0;
}

bool E57Reader::IsReaderValid(int64_t scanIdx)
{
    Data3DPointsDouble* pointsDataPtr = this->mScanDataPoints[scanIdx];
    return pointsDataPtr != nullptr;
}

void E57Reader::DestroyScanReader(int64_t scanIdx)
{
    if (this->mScanDataPoints[scanIdx] != nullptr)
    {
        delete this->mScanDataPoints[scanIdx];
        this->mScanDataPoints[scanIdx] = nullptr;
    }

    this->mScanReaders[scanIdx]->close();
}

E57Reader::~E57Reader()
{
    for (int64_t iScanPts = 0; iScanPts < this->mScanDataPoints.size(); iScanPts++)
    {
        Data3DPointsDouble* pointsDataPtr = this->mScanDataPoints[iScanPts];
        if (pointsDataPtr != nullptr) delete pointsDataPtr;
    }

    if (this->mReader->IsOpen()) this->mReader->Close();
    delete this->mReader;
}

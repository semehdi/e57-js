#include "e57.h"
#include "image_header.h"

E57::E57(const std::string& filePath) {
    this->mReader = new Reader(filePath);
}

E57Root E57::GetHeader()
{
    E57Root e57Root;
    this->mReader->GetE57Root(e57Root);
    return e57Root;
}

Data3D E57::GetData3DHeader(int64_t dataIdx)
{
    Data3D data3DHeader;
    this->mReader->ReadData3D(dataIdx, data3DHeader);
    return data3DHeader;
}

ImageHeader E57::GetImage2DHeader(int64_t imageIdx)
{
    Image2D image2d;
    this->mReader->ReadImage2D(imageIdx, image2d);
    Image2DProjection imageProjection;
    Image2DType imageType;
    int64_t imageWidth, imageHeight, imageSize;
    Image2DType imageMaskType, imageVisualType;
    bool isSucess = this->mReader->GetImage2DSizes(imageIdx, imageProjection, imageType, 
        imageWidth, imageHeight, imageSize, imageMaskType, imageVisualType);
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

int64_t E57::GetData3DCount()
{
    return this->mReader->GetData3DCount();
}

int64_t E57::GetImage2DCount()
{
    return this->mReader->GetImage2DCount();
}

std::vector<Point> E57::ReadScan(int64_t scanIdx, int64_t ptsSize)
{
    std::vector<Point> pts = std::vector<Point>();

    if (ptsSize < 0) return pts;

    const Data3D scanHeader = this->GetData3DHeader(scanIdx);
    const int64_t scanPtsCount = scanHeader.pointCount;
    
    if (this->mReadPtsCount >= scanPtsCount || this->mReadPtsCount < 0)
    {
        this->ResetScanReader(scanIdx);
        return pts;
    }

    ptsSize = std::min(ptsSize, scanPtsCount);
    ptsSize = std::min(scanPtsCount - this->mReadPtsCount, ptsSize);

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
            Point pt = Point(pointsData, i);
            pts[count++] = pt;
            this->mReadPtsCount++;
            if (count >= ptsSize) break;
        }
        if (count >= ptsSize) break;
    }
    return pts;
}

emscripten::val E57::ReadImage(int64_t imageIdx)
{
    Image2DProjection imageProjection;
    Image2DType imageType;
    int64_t imageWidth, imageHeight, imageSize;
    Image2DType imageMaskType, imageVisualType;
    bool isSucess = this->mReader->GetImage2DSizes(imageIdx, imageProjection, imageType, 
        imageWidth, imageHeight, imageSize, imageMaskType, imageVisualType);
    uint8_t* imageData = new uint8_t[imageSize];
    this->mReader->ReadImage2DData(imageIdx, imageProjection, imageType, imageData, 0, imageSize);
    return emscripten::val(emscripten::typed_memory_view(imageSize, imageData));;
}

void E57::MakeScanReader(int64_t scanIdx, int64_t chunkSize)
{
    Data3D scanHeader = this->GetData3DHeader(scanIdx);
    this->mScanDataPoints[scanIdx] = new Data3DPointsDouble(scanHeader);
    this->mScanReaders[scanIdx] = this->mReader->SetUpData3DPointsData(scanIdx, chunkSize, *this->mScanDataPoints[scanIdx]);
}

void E57::ResetScanReader(int64_t scanIdx)
{
    this->DestroyScanReader(scanIdx);
    this->mReadPtsCount = 0;
}

bool E57::IsReaderValid(int64_t scanIdx)
{
    Data3DPointsDouble* pointsDataPtr = this->mScanDataPoints[scanIdx];
    return pointsDataPtr != nullptr;
}

void E57::DestroyScanReader(int64_t scanIdx)
{
    if (this->mScanDataPoints[scanIdx] != nullptr) delete this->mScanDataPoints[scanIdx];
    this->mScanReaders[scanIdx]->close();
}

E57::~E57()
{
    if (this->mReader->IsOpen()) this->mReader->Close();
    delete this->mReader;
}

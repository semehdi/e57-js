#include "e57_reader.hpp"

E57Reader::E57Reader(const std::string& filePath) {
    this->mReader = new Reader(filePath);
}

emscripten::val E57Reader::TestPromise()
{
    auto* p = new EmPromise();
    auto jsPromise = p->take();
    std::thread([p]() {
        emscripten_log(EM_LOG_CONSOLE, "TestPromise: thread running");
        emscripten_async_run_in_main_runtime_thread(EM_FUNC_SIG_VI,
            (void*)(+[](void* arg) {
                auto* p = static_cast<EmPromise*>(arg);
                p->resolve(emscripten::val(42));
                delete p;
            }), p);
    }).detach();
    return jsPromise;
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
    auto* p         = new EmPromise();
    auto  jsPromise = p->take();
    auto* eps       = new EmPromiseSig<uint8_t>{ p };

    std::thread([this, imageIdx, eps]() {
        Image2DProjection proj;
        Image2DType       type, maskType, visType;
        int64_t           w, h, size;

        if (!this->mReader->GetImage2DSizes(imageIdx, proj, type,
                w, h, size, maskType, visType)) {
            eps->error   = "Cannot read image: GetImage2DSizes failed";
            eps->success = false;
        } else {
            eps->size = static_cast<int32_t>(size);
            eps->ptr  = static_cast<uint8_t*>(malloc(eps->size));
            const size_t rBytes = this->mReader->ReadImage2DData(
                imageIdx, proj, type, eps->ptr, 0, eps->size);
            eps->success = (static_cast<int64_t>(rBytes) == size);
            if (!eps->success) {
                free(eps->ptr);
                eps->ptr   = nullptr;
                eps->error = "Cannot read image: incomplete read";
            }
        }

        emscripten_async_run_in_main_runtime_thread(EM_FUNC_SIG_VI,
            (void*)(+[](void* arg) {
                auto* eps = static_cast<EmPromiseSig<uint8_t>*>(arg);
                if (eps->success) {
                    // Zero-copy: Uint8Array views WASM SharedArrayBuffer directly.
                    // FinalizationRegistry in _emjs_uint8array_view frees eps->ptr on GC.
                    eps->promise->resolve(
                        emscripten::val::take_ownership(
                            EmPromise::arrayView<uint8_t>(eps->ptr, eps->size))
                    );
                } else {
                    eps->promise->reject(eps->error);
                }
                delete eps->promise;
                delete eps;
            }), eps);
    }).detach();
    return jsPromise;
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

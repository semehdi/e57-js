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

void E57Reader::FetchScan(int64_t scanIdx, int64_t ptsSize, ScanPromiseSig<Point>& eps)
{
    if (ptsSize < 0)
    {
        eps.success = false;
        eps.error   = "Cannot read scan points !";
        return;
    }

    const Data3D scanHeader = this->GetData3DHeader(scanIdx);
    const int64_t scanPtsCount = scanHeader.pointCount;

    if (this->mReadPtsCount[scanIdx] >= scanPtsCount || this->mReadPtsCount[scanIdx] < 0)
        this->ResetScanReader(scanIdx);

    ptsSize = std::min(ptsSize, scanPtsCount);
    ptsSize = std::min(scanPtsCount - this->mReadPtsCount[scanIdx], ptsSize);

    if (!this->IsReaderValid(scanIdx))
        this->MakeScanReader(scanIdx, ptsSize);

    Data3DPointsDouble* pointsData = this->mScanDataPoints[scanIdx];
    std::shared_ptr<CompressedVectorReader> dataReader = this->mScanReaders[scanIdx];

    eps.vec.reserve(ptsSize);

    int64_t count = 0;
    unsigned size = 0;

    while (size = dataReader->read())
    {
        for (long i = 0; i < size; i++)
        {
            eps.vec.emplace_back(pointsData, i);
            this->mReadPtsCount[scanIdx]++;
            if (++count >= ptsSize) break;
        }
        if (count >= ptsSize) break;
    }
    eps.success = true;
}

emscripten::val E57Reader::ReadScanSync(int64_t scanIdx, int64_t ptsSize)
{
    ScanPromiseSig<Point> eps{ nullptr };
    FetchScan(scanIdx, ptsSize, eps);
    if (!eps.success)
        throw std::runtime_error(eps.error);
    return emscripten::val(std::move(eps.vec));
}

emscripten::val E57Reader::ReadScan(int64_t scanIdx, int64_t ptsSize)
{
    auto* p         = new EmPromise();
    auto  jsPromise = p->take();
    auto* eps       = new ScanPromiseSig<Point>{ p };

    std::thread([this, scanIdx, ptsSize, eps]() {
        try {
            FetchScan(scanIdx, ptsSize, *eps);
        } catch (const std::exception& e) {
            eps->success = false;
            eps->error   = e.what();
        } catch (...) {
            eps->success = false;
            eps->error   = "Unknown error in ReadScan";
        }

        emscripten_async_run_in_main_runtime_thread(EM_FUNC_SIG_VI,
        (void*)(+[](void* arg) {
            auto* eps = static_cast<ScanPromiseSig<Point>*>(arg);
            if (eps->success) {
                eps->promise->resolve(emscripten::val(std::move(eps->vec)));
            } else {
                eps->promise->reject(eps->error);
            }
            delete eps->promise;
            delete eps;
        }), eps);
    }).detach();
    return jsPromise;
}

void E57Reader::FetchImage(int64_t imageIdx, ImagePromiseSig<uint8_t>& eps)
{
    Image2DProjection proj;
    Image2DType       type, maskType, visType;
    int64_t           w, h, size;

    if (!this->mReader->GetImage2DSizes(imageIdx, proj, type,
            w, h, size, maskType, visType)) {
        eps.error   = "Cannot read image: GetImage2DSizes failed";
        eps.success = false;
        return;
    }

    eps.size = static_cast<int32_t>(size);
    eps.ptr  = static_cast<uint8_t*>(malloc(eps.size));
    const size_t rBytes = this->mReader->ReadImage2DData(
        imageIdx, proj, type, eps.ptr, 0, eps.size);
    eps.success = (static_cast<int64_t>(rBytes) == size);
    if (!eps.success) {
        free(eps.ptr);
        eps.ptr   = nullptr;
        eps.error = "Cannot read image: incomplete read";
    }
}

emscripten::val E57Reader::ReadImageSync(int64_t imageIdx)
{
    ImagePromiseSig<uint8_t> eps{ nullptr };
    FetchImage(imageIdx, eps);
    if (!eps.success)
        throw std::runtime_error(eps.error);
    return emscripten::val::take_ownership(
        EmPromise::arrayView<uint8_t>(eps.ptr, eps.size));
}

emscripten::val E57Reader::ReadImage(int64_t imageIdx)
{
    auto* p         = new EmPromise();
    auto  jsPromise = p->take();
    auto* eps       = new ImagePromiseSig<uint8_t>{ p };

    std::thread([this, imageIdx, eps]() {
        try {
            FetchImage(imageIdx, *eps);
        } catch (const std::exception& e) {
            eps->success = false;
            eps->error   = e.what();
        } catch (...) {
            eps->success = false;
            eps->error   = "Unknown error in ReadImage";
        }

        emscripten_async_run_in_main_runtime_thread(EM_FUNC_SIG_VI,
            (void*)(+[](void* arg) {
                auto* eps = static_cast<ImagePromiseSig<uint8_t>*>(arg);
                if (eps->success) {
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
    auto ptsIt = this->mScanDataPoints.find(scanIdx);
    if (ptsIt != this->mScanDataPoints.end() && ptsIt->second != nullptr)
    {
        delete ptsIt->second;
        ptsIt->second = nullptr;
    }

    auto readerIt = this->mScanReaders.find(scanIdx);
    if (readerIt != this->mScanReaders.end())
        readerIt->second->close();
}

E57Reader::~E57Reader()
{
    for (auto& [key, ptr] : this->mScanDataPoints)
        delete ptr;

    if (this->mReader->IsOpen()) this->mReader->Close();
    delete this->mReader;
}

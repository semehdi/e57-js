#include "e57_reader.hpp"

E57Reader::E57Reader(const std::string& filePath) {
    this->mReader = new Reader(filePath);
}

E57Root E57Reader::GetHeader()
{
    E57Root e57Root;
    if (!this->mReader->GetE57Root(e57Root))
        throw std::runtime_error("Cannot read E57 file header");
    return e57Root;
}

Data3D E57Reader::GetData3DHeader(int64_t dataIdx)
{
    Data3D data3DHeader;
    if (!this->mReader->ReadData3D(dataIdx, data3DHeader))
        throw std::runtime_error("Cannot read scan header at index " + std::to_string(dataIdx));
    return data3DHeader;
}

ImageHeader E57Reader::GetImage2DHeader(int64_t imageIdx)
{
    Image2D image2d;
    if (!this->mReader->ReadImage2D(imageIdx, image2d))
        throw std::runtime_error("Cannot read image header at index " + std::to_string(imageIdx));
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

CoordinatesSystem E57Reader::ScanCoordinatesSystem(int64_t scanIdx)
{
    const Data3D scanHeader = this->GetData3DHeader(scanIdx);
    const bool isSpherical = scanHeader.pointFields.sphericalRangeField && scanHeader.pointFields.sphericalAzimuthField && scanHeader.pointFields.sphericalElevationField;
    const bool isCartesian = scanHeader.pointFields.cartesianXField && scanHeader.pointFields.cartesianYField && scanHeader.pointFields.cartesianZField;
    if (isSpherical) return CoordinatesSystem::SPHERICAL;
    if (isCartesian) return CoordinatesSystem::CARTESIAN;
    return CoordinatesSystem::CYLINDRICAL;
}

void E57Reader::mReadScan(int64_t scanIdx, int64_t ptsSize, ScanPromiseSig<Point>& eps, bool transform)
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

    if (!this->mIsReaderValid(scanIdx))
        this->mMakeScanReader(scanIdx, ptsSize);

    Data3DPointsDouble* pointsData = this->mScanDataPoints[scanIdx];
    std::shared_ptr<CompressedVectorReader> dataReader = this->mScanReaders[scanIdx];

    if (!pointsData || !dataReader)
        throw std::runtime_error("Scan reader not initialized for index " + std::to_string(scanIdx));

    eps.vec.reserve(ptsSize);

    const CoordinatesSystem scanSystem = this->ScanCoordinatesSystem(scanIdx);

    int64_t count = 0;
    uint64_t size = 0;

    while (size = dataReader->read())
    {
        for (int64_t i = 0; i < size; i++)
        {
            Point point = Point(pointsData, i);

            if (transform)
            {
                if (scanSystem == CoordinatesSystem::SPHERICAL) point.sphericalToCartesian();
                point.transform(scanHeader.pose);
            }

            eps.vec.emplace_back(point);
            this->mReadPtsCount[scanIdx]++;
            if (++count >= ptsSize) break;
        }
        if (count >= ptsSize) break;
    }
    eps.success = true;
}

emscripten::val E57Reader::ReadScanSync(int64_t scanIdx, int64_t ptsSize, bool transform)
{
    ScanPromiseSig<Point> eps{ nullptr };
    mReadScan(scanIdx, ptsSize, eps, transform);
    if (!eps.success)
        throw std::runtime_error(eps.error);
    return emscripten::val(std::move(eps.vec));
}

emscripten::val E57Reader::ReadScan(int64_t scanIdx, int64_t ptsSize, bool transform)
{
    auto* p         = new EmPromise();
    auto  jsPromise = p->take();
    auto* eps       = new ScanPromiseSig<Point>{ p };

    std::thread([this, scanIdx, ptsSize, eps, transform]() {
        try {
            mReadScan(scanIdx, ptsSize, *eps, transform);
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

void E57Reader::mReadImage(int64_t imageIdx, ImagePromiseSig<uint8_t>& eps)
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
    mReadImage(imageIdx, eps);
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
            mReadImage(imageIdx, *eps);
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

void E57Reader::mMakeScanReader(int64_t scanIdx, int64_t chunkSize)
{
    Data3D scanHeader = this->GetData3DHeader(scanIdx);
    this->mScanDataPoints[scanIdx] = new Data3DPointsDouble(scanHeader);
    auto reader = this->mReader->SetUpData3DPointsData(scanIdx, chunkSize, *this->mScanDataPoints[scanIdx]);
    if (!reader)
        throw std::runtime_error("Cannot set up scan reader at index " + std::to_string(scanIdx));
    this->mScanReaders[scanIdx] = reader;
}

void E57Reader::ResetScanReader(int64_t scanIdx)
{
    this->mDestroyScanReader(scanIdx);
    this->mReadPtsCount[scanIdx] = 0;
}

bool E57Reader::mIsReaderValid(int64_t scanIdx)
{
    Data3DPointsDouble* pointsDataPtr = this->mScanDataPoints[scanIdx];
    return pointsDataPtr != nullptr;
}

void E57Reader::mDestroyScanReader(int64_t scanIdx)
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

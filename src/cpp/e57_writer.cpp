#include "e57_writer.hpp"

E57Writer::E57Writer(const std::string& filePath)
{
    this->mWriter = new Writer(static_cast<ustring>(filePath));
}

static void buildWriteImageSig(WriteImageSig& sig, ImageHeader image2DHeader, Image2DType imageType,
    Image2DProjection imageProjection, int64_t startPos, const emscripten::val& jsArray,
    int64_t byteCount, int32_t width, int32_t height)
{
    sig.data            = emscripten::vecFromJSArray<uint8_t>(jsArray);
    sig.imgHeader       = image2DHeader.ToImage2D();
    sig.imageType       = imageType;
    sig.imageProjection = imageProjection;
    sig.startPos        = startPos;
    sig.byteCount       = byteCount;

    const bool isPng = (imageType == Image2DType::E57_PNG_IMAGE || imageType == Image2DType::E57_PNG_IMAGE_MASK);
    auto setSize = [&](auto& rep) {
        if (isPng) rep.pngImageSize = byteCount; else rep.jpegImageSize = byteCount;
        rep.imageWidth  = width;
        rep.imageHeight = height;
    };
    switch (imageProjection) {
        case Image2DProjection::ProjectionSpherical:   setSize(sig.imgHeader.sphericalRepresentation);   break;
        case Image2DProjection::ProjectionPinhole:     setSize(sig.imgHeader.pinholeRepresentation);     break;
        case Image2DProjection::ProjectionCylindrical: setSize(sig.imgHeader.cylindricalRepresentation); break;
        default:                                       setSize(sig.imgHeader.visualReferenceRepresentation); break;
    }
}

int64_t E57Writer::AddImageSync(
    ImageHeader image2DHeader,
    Image2DType imageType,
    Image2DProjection imageProjection,
    int64_t startPos,
    const emscripten::val& jsArray,
    int64_t byteCount,
    int32_t width,
    int32_t height)
{
    WriteImageSig sig{ nullptr };
    buildWriteImageSig(sig, image2DHeader, imageType, imageProjection, startPos, jsArray, byteCount, width, height);
    FetchAddImage(sig);
    return sig.result;
}

void E57Writer::FetchAddImage(WriteImageSig& sig)
{
    const size_t wBytes = this->mWriter->WriteImage2DData(
        sig.imgHeader, sig.imageType, sig.imageProjection,
        sig.startPos, sig.data.data(), sig.byteCount);
    sig.result  = static_cast<int64_t>(wBytes);
    sig.success = true;
}

emscripten::val E57Writer::AddImage(
    ImageHeader image2DHeader,
    Image2DType imageType,
    Image2DProjection imageProjection,
    int64_t startPos,
    const emscripten::val& jsArray,
    int64_t byteCount,
    int32_t width,
    int32_t height)
{
    auto* p         = new EmPromise();
    auto  jsPromise = p->take();
    auto* sig       = new WriteImageSig{ p };

    buildWriteImageSig(*sig, image2DHeader, imageType, imageProjection, startPos, jsArray, byteCount, width, height);

    std::thread([this, sig]() {
        try {
            FetchAddImage(*sig);
        } catch (const std::exception& e) {
            sig->success = false;
            sig->error   = e.what();
        } catch (...) {
            sig->success = false;
            sig->error   = "Unknown error in AddImage";
        }

        emscripten_async_run_in_main_runtime_thread(EM_FUNC_SIG_VI,
            (void*)(+[](void* arg) {
                auto* sig = static_cast<WriteImageSig*>(arg);
                if (sig->success)
                    sig->promise->resolve(emscripten::val(sig->result));
                else
                    sig->promise->reject(sig->error);
                delete sig->promise;
                delete sig;
            }), sig);
    }).detach();
    return jsPromise;
}

static void fillPointsData(Data3DPointsDouble& pointsData, const emscripten::val& ptsArray, int64_t numPoints)
{
    for (int64_t i = 0; i < numPoints; ++i)
    {
        const Point pt = ptsArray[i].as<Point>();

        pointsData.cartesianX[i]            = pt.cartesianX;
        pointsData.cartesianY[i]            = pt.cartesianY;
        pointsData.cartesianZ[i]            = pt.cartesianZ;
        pointsData.cartesianInvalidState[i] = pt.cartesianInvalidState;

        pointsData.timeStamp[i]             = pt.timeStamp;
        pointsData.isTimeStampInvalid[i]    = pt.isTimeStampInvalid;

        pointsData.colorRed[i]              = pt.colorRed;
        pointsData.colorGreen[i]            = pt.colorGreen;
        pointsData.colorBlue[i]             = pt.colorBlue;
        pointsData.isColorInvalid[i]        = pt.isColorInvalid;

        pointsData.normalX[i]               = pt.normalX;
        pointsData.normalY[i]               = pt.normalY;
        pointsData.normalZ[i]               = pt.normalZ;

        pointsData.sphericalAzimuth[i]      = pt.sphericalAzimuth;
        pointsData.sphericalElevation[i]    = pt.sphericalElevation;
        pointsData.sphericalRange[i]        = pt.sphericalRange;

        pointsData.returnCount[i]           = pt.returnCount;
        pointsData.returnIndex[i]           = pt.returnIndex;

        pointsData.columnIndex[i]           = pt.columnIndex;
        pointsData.rowIndex[i]              = pt.rowIndex;

        pointsData.intensity[i]             = pt.intensity;
        pointsData.isIntensityInvalid[i]    = pt.isIntensityInvalid;
    }
}

void E57Writer::FetchAddScan(WriteScanSig& sig)
{
    sig.result  = this->mWriter->WriteData3DData(sig.header, *sig.pointsData);
    sig.success = true;
}

int64_t E57Writer::AddScanSync(Data3D header, const emscripten::val& ptsArray)
{
    const int64_t numPoints = ptsArray["length"].as<int64_t>();
    header.pointCount = numPoints;

    Data3DPointsDouble pointsData(header);
    fillPointsData(pointsData, ptsArray, numPoints);

    return this->mWriter->WriteData3DData(header, pointsData);
}

emscripten::val E57Writer::AddScan(Data3D header, const emscripten::val& ptsArray)
{
    auto* p         = new EmPromise();
    auto  jsPromise = p->take();
    auto* sig       = new WriteScanSig{ p };

    // Extract data on main thread — emscripten::val access required here
    const int64_t numPoints = ptsArray["length"].as<int64_t>();
    header.pointCount  = numPoints;
    sig->header        = header;
    sig->pointsData    = new Data3DPointsDouble(header);
    fillPointsData(*sig->pointsData, ptsArray, numPoints);

    std::thread([this, sig]() {
        try {
            FetchAddScan(*sig);
        } catch (const std::exception& e) {
            sig->success = false;
            sig->error   = e.what();
        } catch (...) {
            sig->success = false;
            sig->error   = "Unknown error in AddScan";
        }

        emscripten_async_run_in_main_runtime_thread(EM_FUNC_SIG_VI,
            (void*)(+[](void* arg) {
                auto* sig = static_cast<WriteScanSig*>(arg);
                if (sig->success)
                    sig->promise->resolve(emscripten::val(sig->result));
                else
                    sig->promise->reject(sig->error);
                delete sig->pointsData;
                delete sig->promise;
                delete sig;
            }), sig);
    }).detach();
    return jsPromise;
}

void E57Writer::Close()
{
    if (this->mWriter != nullptr)
    {
        this->mWriter->Close();
        delete this->mWriter;
        this->mWriter = nullptr;
    }
}

E57Writer::~E57Writer()
{
    this->Close();
}
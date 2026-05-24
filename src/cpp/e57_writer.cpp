#include "e57_writer.hpp"

E57Writer::E57Writer(const std::string& filePath)
{
    this->mWriter = new Writer(static_cast<ustring>(filePath));
    std::cout << std::to_string(this->mWriter->IsOpen()) << std::endl;
}

int64_t E57Writer::AddImage(
    ImageHeader image2DHeader, 
    Image2DType imageType, 
    Image2DProjection imageProjection, 
    int64_t startPos, 
    const emscripten::val& jsArray, 
    int64_t byteCount,
    int32_t width,
    int32_t height
)
{
    const std::vector<uint8_t> data = emscripten::vecFromJSArray<uint8_t>(jsArray);
    Image2D imgHeader = image2DHeader.ToImage2D();

    switch (imageProjection)
    {
        case Image2DProjection::ProjectionVisual:
            if (imageType == Image2DType::E57_PNG_IMAGE || imageType == Image2DType::E57_PNG_IMAGE_MASK)
                imgHeader.visualReferenceRepresentation.pngImageSize = byteCount;
            else
                imgHeader.visualReferenceRepresentation.jpegImageSize = byteCount;
            
            imgHeader.visualReferenceRepresentation.imageWidth = width;
            imgHeader.visualReferenceRepresentation.imageHeight = height;
            break;
        case Image2DProjection::ProjectionSpherical:
            if (imageType == Image2DType::E57_PNG_IMAGE || imageType == Image2DType::E57_PNG_IMAGE_MASK)
                imgHeader.sphericalRepresentation.pngImageSize = byteCount;
            else
                imgHeader.sphericalRepresentation.jpegImageSize = byteCount;

            imgHeader.sphericalRepresentation.imageWidth = width;
            imgHeader.sphericalRepresentation.imageHeight = height;
            break;
        case Image2DProjection::ProjectionPinhole:
            if (imageType == Image2DType::E57_PNG_IMAGE || imageType == Image2DType::E57_PNG_IMAGE_MASK)
                imgHeader.pinholeRepresentation.pngImageSize = byteCount;
            else
                imgHeader.pinholeRepresentation.jpegImageSize = byteCount;
            
            imgHeader.pinholeRepresentation.imageWidth = width;
            imgHeader.pinholeRepresentation.imageHeight = height;
            break;
        case Image2DProjection::ProjectionCylindrical:
            if (imageType == Image2DType::E57_PNG_IMAGE || imageType == Image2DType::E57_PNG_IMAGE_MASK)
                imgHeader.cylindricalRepresentation.pngImageSize = byteCount;
            else
                imgHeader.cylindricalRepresentation.jpegImageSize = byteCount;
            
            imgHeader.cylindricalRepresentation.imageWidth = width;
            imgHeader.cylindricalRepresentation.imageHeight = height;
            break;
        default:
            if (imageType == Image2DType::E57_PNG_IMAGE || imageType == Image2DType::E57_PNG_IMAGE_MASK)
                imgHeader.visualReferenceRepresentation.pngImageSize = byteCount;
            else
                imgHeader.visualReferenceRepresentation.jpegImageSize = byteCount;
            
            imgHeader.visualReferenceRepresentation.imageWidth = width;
            imgHeader.visualReferenceRepresentation.imageHeight = height;
    }

    const int64_t wBytes = this->mWriter->WriteImage2DData(imgHeader, imageType, imageProjection, startPos, data.data(), byteCount);
    return wBytes;
}

int64_t E57Writer::AddScan(
        Data3D header,
        const emscripten::val& ptsArray
    )
{
    const std::vector<Point> points = emscripten::vecFromJSArray<Point>(ptsArray);

    const int64_t numPoints = points.size();
    header.pointCount = numPoints;

    e57::Data3DPointsDouble pointsData( header );

    for ( int64_t iPoint = 0; iPoint < numPoints; ++iPoint )
    {
        const Point point = points[iPoint];

        pointsData.cartesianX[iPoint] = point.cartesianX;
        pointsData.cartesianY[iPoint] = point.cartesianY;
        pointsData.cartesianZ[iPoint] = point.cartesianZ;
        pointsData.cartesianInvalidState[iPoint] = point.cartesianInvalidState;

        pointsData.timeStamp[iPoint] = point.timeStamp;
        pointsData.isTimeStampInvalid[iPoint] = point.isTimeStampInvalid;

        pointsData.colorRed[iPoint] = point.colorRed;
        pointsData.colorGreen[iPoint] = point.colorGreen;
        pointsData.colorBlue[iPoint] = point.colorBlue;
        pointsData.isColorInvalid[iPoint] = point.isColorInvalid;

        pointsData.normalX[iPoint] = point.normalX;
        pointsData.normalY[iPoint] = point.normalY;
        pointsData.normalZ[iPoint] = point.normalZ;

        pointsData.sphericalAzimuth[iPoint] = point.sphericalAzimuth;
        pointsData.sphericalElevation[iPoint] = point.sphericalElevation;
        pointsData.sphericalRange[iPoint] = point.sphericalRange;

        pointsData.returnCount[iPoint] = point.returnCount;
        pointsData.returnIndex[iPoint] = point.returnIndex;

        pointsData.columnIndex[iPoint] = point.columnIndex;
        pointsData.rowIndex[iPoint] = point.rowIndex;

        pointsData.intensity[iPoint] = point.intensity;
        pointsData.isIntensityInvalid[iPoint] = point.isIntensityInvalid;
    }

    const int64_t scanIdx = this->mWriter->WriteData3DData( header, pointsData );

    return scanIdx;
}

void E57Writer::Close()
{
    if (this->mWriter != nullptr)
    {
        this->mWriter->Close();
        delete this->mWriter;
    }
}

E57Writer::~E57Writer()
{
    this->Close();
}
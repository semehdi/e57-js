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
    std::vector<uint8_t> data = emscripten::vecFromJSArray<uint8_t>(jsArray);
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
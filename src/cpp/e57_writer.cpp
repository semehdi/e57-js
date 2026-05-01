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
    int64_t byteCount
)
{
    std::vector<uint8_t> data = emscripten::vecFromJSArray<uint8_t>(jsArray);
    std::cout << "The size" << std::endl; 
    std::cout << data.size() << std::endl;
    std::cout << byteCount << std::endl;
    std::cout << std::to_string(this->mWriter->IsOpen()) << std::endl;
    Image2D imgHeader = image2DHeader.ToImage2D();
    imgHeader.visualReferenceRepresentation.jpegImageSize = byteCount;
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
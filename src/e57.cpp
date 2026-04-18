#include "e57.h"

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
    this->mReader->ReadData3D(dataIdx, data3DHeader );
    return data3DHeader;
}

int64_t E57::GetData3DCount()
{
    return this->mReader->GetData3DCount();
}

int64_t E57::GetImage2DCount()
{
    return this->mReader->GetImage2DCount();
}

E57::~E57()
{
    if (this->mReader->IsOpen()) this->mReader->Close();
    delete this->mReader;
}

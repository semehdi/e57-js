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

std::vector<Point> E57::ReadData(int64_t scanIdx)
{
    std::vector<Point> pts = std::vector<Point>();
    Data3D scanHeader = this->GetData3DHeader(scanIdx);
    Data3DPointsDouble* pointsData = new Data3DPointsDouble(scanHeader);
    CompressedVectorReader dataReader = this->mReader->SetUpData3DPointsData(scanIdx, scanHeader.pointCount, *pointsData);

    int64_t count = 0;
    unsigned size = 0;
    int col = 0;
    int row = 0;
    double intensity = 0;
    uint16_t red = 0, green = 0, blue = 0;

    while (size = dataReader.read())
    {
        for (long i = 0; i < size; i++)
        {
            Point pt = Point(pointsData, i);
            pts.push_back(pt);
            count++;
        }
    }
    dataReader.close();
    return pts;
}

E57::~E57()
{
    if (this->mReader->IsOpen()) this->mReader->Close();
    delete this->mReader;
}

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
    int64_t nColumn = 0;    
    int64_t nRow = 0;
    int64_t nPointsSize = 0;
    int64_t nGroupsSize = 0;
    int64_t nCountSize = 0;
    bool bColumnIndex = false;

    std::vector<Point> pts = std::vector<Point>();

    Data3D scanHeader = this->GetData3DHeader(scanIdx);

    this->mReader->GetData3DSizes(scanIdx, nRow, nColumn, nPointsSize, nGroupsSize, nCountSize, bColumnIndex);

    int64_t nSize = nRow;
    if (nSize == 0) nSize = 1024;

    bool bIntensity = false;
    double intRange = 0;
    double intOffset = 0;

    if(scanHeader.pointFields.intensityField)
    {
        bIntensity = true;
        intRange = scanHeader.intensityLimits.intensityMaximum - scanHeader.intensityLimits.intensityMinimum;
        intOffset = scanHeader.intensityLimits.intensityMinimum;
    }

    std::vector<int64_t> idElementValue = std::vector<int64_t>();
    std::vector<int64_t> startPointIndex = std::vector<int64_t>();
    std::vector<int64_t> pointCount = std::vector<int64_t>();
    if(nGroupsSize > 0)
    {
        idElementValue.resize(nGroupsSize);
        startPointIndex.resize(nGroupsSize);
        pointCount.resize(nGroupsSize);

        if(!this->mReader->ReadData3DGroupsData(scanIdx, nGroupsSize, idElementValue.data(), startPointIndex.data(), pointCount.data()))
            nGroupsSize = 0;
    }

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

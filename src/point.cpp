#include "point.h"

Point::Point(Data3DPointsDouble* pointData, int64_t idx)
{
    this->cartesianX = pointData->cartesianX[idx];
    this->cartesianY = pointData->cartesianY[idx];
    this->cartesianZ = pointData->cartesianZ[idx];
    this->cartesianInvalidState = pointData->cartesianInvalidState[idx];
    this->colorRed = pointData->colorRed[idx];
    this->colorGreen = pointData->colorGreen[idx];
    this->colorBlue = pointData->colorBlue[idx];
    this->isColorInvalid = pointData->isColorInvalid[idx];
    this->columnIndex = pointData->columnIndex[idx];
    this->rowIndex = pointData->rowIndex[idx];
    this->intensity = pointData->intensity[idx];
    this->isIntensityInvalid = pointData->isIntensityInvalid[idx];
    this->timeStamp = pointData->timeStamp[idx];
    this->isTimeStampInvalid = pointData->isTimeStampInvalid[idx];
    this->normalX = pointData->normalX[idx];
    this->normalY = pointData->normalY[idx];
    this->normalZ = pointData->normalZ[idx];
    this->returnCount = pointData->returnCount[idx];
    this->returnIndex = pointData->returnIndex[idx];
    this->sphericalAzimuth = pointData->sphericalAzimuth[idx];
    this->sphericalElevation = pointData->sphericalElevation[idx];
    this->sphericalRange = pointData->sphericalRange[idx];
}

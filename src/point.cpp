#include "point.h"

Point::Point(Data3DPointsDouble* pointData, int64_t idx)
{
    this->cartesianX = pointData->cartesianX[idx];
    this->cartesianY = pointData->cartesianY[idx];
    this->cartesianZ = pointData->cartesianZ[idx];
    this->cartesianInvalidState = pointData->cartesianInvalidState[idx];
}

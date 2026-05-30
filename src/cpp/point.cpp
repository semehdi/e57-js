#include "point.hpp"

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

Point::Point() {}

void Point::transform(const RigidBodyTransform& t)
{
    const double qw = t.rotation.w;
    const double qx = t.rotation.x;
    const double qy = t.rotation.y;
    const double qz = t.rotation.z;

    // q × v
    const double cx = qy * cartesianZ - qz * cartesianY;
    const double cy = qz * cartesianX - qx * cartesianZ;
    const double cz = qx * cartesianY - qy * cartesianX;

    // q × (q × v)
    const double ccx = qy * cz - qz * cy;
    const double ccy = qz * cx - qx * cz;
    const double ccz = qx * cy - qy * cx;

    cartesianX += 2.0 * (qw * cx + ccx) + t.translation.x;
    cartesianY += 2.0 * (qw * cy + ccy) + t.translation.y;
    cartesianZ += 2.0 * (qw * cz + ccz) + t.translation.z;
}

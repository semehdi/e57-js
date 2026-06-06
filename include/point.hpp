#pragma once

#include "E57SimpleData.h"

using namespace e57;

class Point
{
public:
    double cartesianX, cartesianY, cartesianZ, intensity, timeStamp;
    double sphericalRange, sphericalAzimuth, sphericalElevation;
    int8_t cartesianInvalidState, isIntensityInvalid, isColorInvalid, returnIndex, returnCount;
    int8_t isTimeStampInvalid;
    uint16_t colorRed, colorGreen, colorBlue;
    int32_t rowIndex, columnIndex;
    float normalX, normalY, normalZ;

    Point(Data3DPointsDouble* pointData, int64_t idx);
    Point();

    void transform(const RigidBodyTransform& t);

    void cartesianToSpherical();
    void sphericalToCartesian();
};
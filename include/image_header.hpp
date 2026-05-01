#pragma once

#include "E57SimpleReader.h"
#include "E57SimpleData.h"

using namespace e57;

class ImageHeader
{
public:
      std::string name;
      std::string guid;
      std::string description;
      DateTime acquisitionDateTime;
      std::string associatedData3DGuid;
      std::string sensorVendor;
      std::string sensorModel;
      std::string sensorSerialNumber;
      Image2DType imageType;
      Image2DProjection imageProjection;
      int64_t width;
      int64_t height;
      Image2DType imageMaskType;
      Image2DType imageVisualType;
      int64_t imageSize;

      RigidBodyTransform pose;
      VisualReferenceRepresentation visualReferenceRepresentation;
      PinholeRepresentation pinholeRepresentation;
      SphericalRepresentation sphericalRepresentation;
      CylindricalRepresentation cylindricalRepresentation;
      Extension::PinholeCameraDistortion pinholeCameraDistortionExt;

      ImageHeader();
      ImageHeader(const Image2D& image2d);
      Image2D ToImage2D();
};

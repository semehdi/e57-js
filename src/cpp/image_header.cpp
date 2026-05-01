#include "image_header.hpp"

ImageHeader::ImageHeader() {};

ImageHeader::ImageHeader(const Image2D& image2D)
{
    this->acquisitionDateTime = image2D.acquisitionDateTime;
    this->associatedData3DGuid = image2D.associatedData3DGuid;
    this->cylindricalRepresentation = image2D.cylindricalRepresentation;
    this->description = image2D.description;
    this->guid = image2D.guid;
    this->name = image2D.name;
    this->pinholeRepresentation = image2D.pinholeRepresentation;
    this->pose = image2D.pose;
    this->sensorModel = image2D.sensorModel;
    this->sensorSerialNumber = image2D.sensorSerialNumber;
    this->sensorVendor = image2D.sensorVendor;
    this->sphericalRepresentation = image2D.sphericalRepresentation;
    this->visualReferenceRepresentation = image2D.visualReferenceRepresentation;

    this->pinholeCameraDistortionExt.cameraNumber = image2D.pinholeCameraDistortionExt->cameraNumber;
    this->pinholeCameraDistortionExt.CV_CX        = image2D.pinholeCameraDistortionExt->CV_CX;
    this->pinholeCameraDistortionExt.CV_CY        = image2D.pinholeCameraDistortionExt->CV_CY;
    this->pinholeCameraDistortionExt.CV_FX        = image2D.pinholeCameraDistortionExt->CV_FX;
    this->pinholeCameraDistortionExt.CV_FY        = image2D.pinholeCameraDistortionExt->CV_FY;
    this->pinholeCameraDistortionExt.CV_HEIGHT    = image2D.pinholeCameraDistortionExt->CV_HEIGHT;
    this->pinholeCameraDistortionExt.CV_K1        = image2D.pinholeCameraDistortionExt->CV_K1;
    this->pinholeCameraDistortionExt.CV_K2        = image2D.pinholeCameraDistortionExt->CV_K2;
    this->pinholeCameraDistortionExt.CV_K3        = image2D.pinholeCameraDistortionExt->CV_K3;
    this->pinholeCameraDistortionExt.CV_K4        = image2D.pinholeCameraDistortionExt->CV_K4;
    this->pinholeCameraDistortionExt.CV_K5        = image2D.pinholeCameraDistortionExt->CV_K5;
    this->pinholeCameraDistortionExt.CV_K6        = image2D.pinholeCameraDistortionExt->CV_K6;
    this->pinholeCameraDistortionExt.CV_P1        = image2D.pinholeCameraDistortionExt->CV_P1;
    this->pinholeCameraDistortionExt.CV_P2        = image2D.pinholeCameraDistortionExt->CV_P2;
    this->pinholeCameraDistortionExt.CV_WIDTH     = image2D.pinholeCameraDistortionExt->CV_WIDTH;
    this->pinholeCameraDistortionExt.type         = image2D.pinholeCameraDistortionExt->type;
}

Image2D ImageHeader::ToImage2D()
{
    std::unique_ptr<Extension::PinholeCameraDistortion> camDistorsion = std::make_unique<Extension::PinholeCameraDistortion>();
    
    camDistorsion->cameraNumber = this->pinholeCameraDistortionExt.cameraNumber;
    camDistorsion->CV_CX        = this->pinholeCameraDistortionExt.CV_CX;
    camDistorsion->CV_CY        = this->pinholeCameraDistortionExt.CV_CY;
    camDistorsion->CV_FX        = this->pinholeCameraDistortionExt.CV_FX;
    camDistorsion->CV_FY        = this->pinholeCameraDistortionExt.CV_FY;
    camDistorsion->CV_HEIGHT    = this->pinholeCameraDistortionExt.CV_HEIGHT;
    camDistorsion->CV_K1        = this->pinholeCameraDistortionExt.CV_K1;
    camDistorsion->CV_K2        = this->pinholeCameraDistortionExt.CV_K2;
    camDistorsion->CV_K3        = this->pinholeCameraDistortionExt.CV_K3;
    camDistorsion->CV_K4        = this->pinholeCameraDistortionExt.CV_K4;
    camDistorsion->CV_K5        = this->pinholeCameraDistortionExt.CV_K5;
    camDistorsion->CV_K6        = this->pinholeCameraDistortionExt.CV_K6;
    camDistorsion->CV_P1        = this->pinholeCameraDistortionExt.CV_P1;
    camDistorsion->CV_P2        = this->pinholeCameraDistortionExt.CV_P2;
    camDistorsion->CV_WIDTH     = this->pinholeCameraDistortionExt.CV_WIDTH;
    camDistorsion->type         = this->pinholeCameraDistortionExt.type;

    return {
        this->name,
        this->guid,
        this->description,
        this->acquisitionDateTime,
        this->associatedData3DGuid,
        this->sensorVendor,
        this->sensorModel,
        this->sensorSerialNumber,
        this->pose,
        this->visualReferenceRepresentation,
        this->pinholeRepresentation,
        this->sphericalRepresentation,
        this->cylindricalRepresentation,
        std::move(camDistorsion)
    };
}

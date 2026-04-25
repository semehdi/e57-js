#include "image_header.h"

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
    this->pinholeCameraDistortionExt.CV_CX = image2D.pinholeCameraDistortionExt->CV_CX;
    this->pinholeCameraDistortionExt.CV_CY = image2D.pinholeCameraDistortionExt->CV_CY;
    this->pinholeCameraDistortionExt.CV_FX = image2D.pinholeCameraDistortionExt->CV_FX;
    this->pinholeCameraDistortionExt.CV_FY = image2D.pinholeCameraDistortionExt->CV_FY;
    this->pinholeCameraDistortionExt.CV_HEIGHT = image2D.pinholeCameraDistortionExt->CV_HEIGHT;
    this->pinholeCameraDistortionExt.CV_K1 = image2D.pinholeCameraDistortionExt->CV_K1;
    this->pinholeCameraDistortionExt.CV_K2 = image2D.pinholeCameraDistortionExt->CV_K2;
    this->pinholeCameraDistortionExt.CV_K3 = image2D.pinholeCameraDistortionExt->CV_K3;
    this->pinholeCameraDistortionExt.CV_K4 = image2D.pinholeCameraDistortionExt->CV_K4;
    this->pinholeCameraDistortionExt.CV_K5 = image2D.pinholeCameraDistortionExt->CV_K5;
    this->pinholeCameraDistortionExt.CV_K6 = image2D.pinholeCameraDistortionExt->CV_K6;
    this->pinholeCameraDistortionExt.CV_P1 = image2D.pinholeCameraDistortionExt->CV_P1;
    this->pinholeCameraDistortionExt.CV_P2 = image2D.pinholeCameraDistortionExt->CV_P2;
    this->pinholeCameraDistortionExt.CV_WIDTH = image2D.pinholeCameraDistortionExt->CV_WIDTH;
    this->pinholeCameraDistortionExt.type = image2D.pinholeCameraDistortionExt->type;
}

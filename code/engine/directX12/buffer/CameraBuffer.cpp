#include "directX12/buffer/CameraBuffer.h"

#include "System.h"
#include "directX12/dxFunctionHelper/DxFunctionHelper.h"

void CameraBuffer::Init(){
	DxFH::CreateBufferResource(System::getInstance()->getDxDevice(),buff_,sizeof(CameraBuffer::ConstantBuffer));
	buff_->Map(0,nullptr,reinterpret_cast<void **>(&mappingData_));
}

void CameraBuffer::Finalize(){
	buff_.Reset();
}

void CameraBuffer::Update(){
	viewMat = MakeMatrix::Affine({1.0f,1.0f,1.0f},rotate,translate);
	cameraPos_ = viewMat[3];
	viewMat = viewMat.Inverse();

	projectionMat = MakeMatrix::PerspectiveFov(fovAngleY,aspectRatio,nearZ,farZ);
}

void CameraBuffer::ConvertToBuffer(){
	mappingData_->cameraPos = translate;
	mappingData_->view = viewMat;
	mappingData_->viewTranspose = viewMat.Transpose();
	mappingData_->projection = projectionMat;
}
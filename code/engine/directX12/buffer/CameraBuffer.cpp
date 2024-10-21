#include "directX12/buffer/CameraBuffer.h"

#include "System.h"
#include "directX12/dxFunctionHelper/DxFunctionHelper.h"

void CameraBuffer::Init(){
	buff_.CreateBufferResource(System::getInstance()->getDxDevice(),sizeof(CameraBuffer::ConstantBuffer));
	buff_.getResource()->Map(0,nullptr,reinterpret_cast<void **>(&mappingData_));
}

void CameraBuffer::Finalize(){
	buff_.Finalize();
}

void CameraBuffer::UpdateMatrix(){
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
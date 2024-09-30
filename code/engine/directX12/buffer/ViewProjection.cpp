#include "directX12/buffer/ViewProjection.h"

#include "System.h"
#include "directX12/dxFunctionHelper/DxFunctionHelper.h"

void ViewProjection::Init(){
	DxFH::CreateBufferResource(System::getInstance()->getDxDevice(),buff_,sizeof(ConstBufferDataViewProjection));
	buff_->Map(0,nullptr,reinterpret_cast<void **>(&mappingData_));
}

void ViewProjection::Finalize(){
	buff_.Reset();
}

void ViewProjection::UpdateMatrix(){
	viewMat = MakeMatrix::Affine({1.0f,1.0f,1.0f},rotate,translate);
	cameraPos_ = viewMat[3];
	viewMat = viewMat.Inverse();

	projectionMat = MakeMatrix::PerspectiveFov(fovAngleY,aspectRatio,nearZ,farZ);
}

void ViewProjection::ConvertToBuffer(){
	mappingData_->cameraPos = translate;
	mappingData_->view = viewMat;
	mappingData_->viewTranspose = viewMat.Transpose();
	mappingData_->projection = projectionMat;
}

void ViewProjection::SetForRootParameter(ID3D12GraphicsCommandList *cmdList,UINT rootParameterNum)const{
	cmdList->SetGraphicsRootConstantBufferView(rootParameterNum,buff_->GetGPUVirtualAddress());
}
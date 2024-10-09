#include "directX12/buffer/TransformBuffer.h"

#include "directX12/dxFunctionHelper/DxFunctionHelper.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

#include <System.h>

void TransformBuffer::Init(){
	DxFH::CreateBufferResource(System::getInstance()->getDxDevice(),buff_,sizeof(TransformBuffer::ConstantBuffer));
	buff_->Map(0,nullptr,reinterpret_cast<void **>(&mappingWorldMat_));

	worldMat = MakeMatrix::Identity();
	mappingWorldMat_->world = worldMat;
}

void TransformBuffer::Finalize(){
	buff_.Reset();
}

void TransformBuffer::Update(){
	worldMat = MakeMatrix::Affine(scale,rotate,translate);

	if(parent != nullptr){
		worldMat *= parent->worldMat;
	}
	ConvertToBuffer();
}

void TransformBuffer::Debug(const std::string &transformName){
	std::string labelName = transformName + " scale";
	ImGui::DragFloat3(labelName.c_str(),&scale.x,0.01f);
	labelName = transformName + " rotate";
	ImGui::DragFloat3(labelName.c_str(),&rotate.x,0.01f);
	labelName = transformName + " translation";
	ImGui::DragFloat3(labelName.c_str(),&translate.x,0.1f);
}

void TransformBuffer::ConvertToBuffer(){
	mappingWorldMat_->world = worldMat;
}
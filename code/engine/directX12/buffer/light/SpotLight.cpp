#include "directX12/buffer/light/SpotLight.h"

#include "directX12/dxFunctionHelper/DxFunctionHelper.h"
#include "imgui/imgui.h"
#include "System.h"

void SpotLight::Init(){
	mappingData_ = nullptr;

	DxFH::CreateBufferResource(System::getInstance()->getDxDevice(),buff_,sizeof(SpotLight::ConstantBuffer));

	buff_->Map(
		0,nullptr,reinterpret_cast<void **>(&mappingData_)
	);
}

void SpotLight::Finalize(){
	buff_.Reset();
}

void SpotLight::DebugUpdate(){
#ifdef _DEBUG
	if(ImGui::Begin("SpotLight")){
		ImGui::DragFloat3("Position",&this->pos.x,0.01f);
		ImGui::ColorEdit4("Color",&this->color.x);
		ImGui::SliderFloat("Intensity",&this->intensity,0.0f,1.0f);
		ImGui::DragFloat3("Direction",&this->direction.x,0.1f);
		this->direction = direction.Normalize();
		ImGui::DragFloat("Distance",&this->distance,0.1f,0.1f,FLT_MAX);
		ImGui::DragFloat("Decay",&this->decay,0.1f,0.1f,FLT_MAX);
		ImGui::DragFloat("CosAngle",&this->cosAngle,0.01f,0.0f,FLT_MAX);
		ImGui::DragFloat("CosFalloffStart",&this->cosFalloffStart,0.1f);
		this->ConvertToBuffer();
	}
	ImGui::End();
#endif // _DEBUG
}

void SpotLight::ConvertToBuffer(){
	mappingData_->color     = this->color;
	mappingData_->pos       = this->pos;
	mappingData_->intensity = this->intensity;
	mappingData_->direction = this->direction;
	mappingData_->distance  = this->distance;
	mappingData_->decay     = this->decay;
	mappingData_->cosAngle  = this->cosAngle;
	mappingData_->cosFalloffStart = this->cosFalloffStart;
}

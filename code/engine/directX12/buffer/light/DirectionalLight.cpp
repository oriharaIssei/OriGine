#include "directX12/buffer/light/DirectionalLight.h"

#include "directX12/dxFunctionHelper/DxFunctionHelper.h"
#include "System.h"

#include "imgui/imgui.h"

void DirectionalLight::Init(){
	mappingData_ = nullptr;

	DxFH::CreateBufferResource(System::getInstance()->getDxDevice(),buff_,sizeof(DirectionalLight::ConstantBuffer));

	buff_->Map(
		0,nullptr,reinterpret_cast<void**>(&mappingData_)
	);
}

void DirectionalLight::Finalize(){
	buff_.Reset();
}

void DirectionalLight::DebugUpdate(){
#ifdef _DEBUG
	if(ImGui::Begin("DirectionalLight")){
		ImGui::DragFloat3("Direction",&this->direction.x,0.01f,-1.0f,1.0f);
		this->direction = this->direction.Normalize();
		ImGui::ColorEdit3("Color",&this->color.x);
		ImGui::SliderFloat("Intensity",&this->intensity,0.0f,1.0f);
		this->ConvertToBuffer();
	}
	ImGui::End();
#endif // _DEBUG
}


void DirectionalLight::ConvertToBuffer(){
	mappingData_->color = color;
	mappingData_->direction = direction;
	mappingData_->intensity = intensity;
}

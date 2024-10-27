#include "directX12/buffer/light/PointLight.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

void PointLight::DebugUpdate(){
#ifdef _DEBUG
	ImGui::Begin("PointLight");
	ImGui::DragFloat3("Position",&this->pos.x,0.01f);
	ImGui::ColorEdit3("Color",&this->color.x);
	ImGui::SliderFloat("Intensity",&this->intensity,0.0f,1.0f);
	ImGui::DragFloat("Radius",&this->radius,0.1f,0.1f,FLT_MAX);
	ImGui::DragFloat("Decay",&this->decay,0.1f,0.1f,FLT_MAX);
	ImGui::End();
#endif // _DEBUG
}
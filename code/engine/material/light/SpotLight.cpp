#include "material/light/SpotLight.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

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
	}
	ImGui::End();
#endif // _DEBUG
}
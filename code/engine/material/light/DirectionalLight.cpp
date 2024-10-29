#include "material/light/DirectionalLight.h"

#include "directX12/DxFunctionHelper.h"
#include "System.h"

#include "imgui/imgui.h"


void DirectionalLight::DebugUpdate(){
#ifdef _DEBUG
	if(ImGui::Begin("DirectionalLight")){
		ImGui::DragFloat3("Direction",&this->direction.x,0.01f,-1.0f,1.0f);
		this->direction = this->direction.Normalize();
		ImGui::ColorEdit3("Color",&this->color.x);
		ImGui::SliderFloat("Intensity",&this->intensity,0.0f,1.0f);
	}
	ImGui::End();
#endif // _DEBUG
}
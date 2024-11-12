#include "material/light/PointLight.h"

#include "globalVariables/GlobalVariables.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

void PointLight::Init(int32_t num){
	GlobalVariables* variables = GlobalVariables::getInstance();

	variables->addValue("LightManager","PointLight" + std::to_string(num),"pos",pos);
	variables->addValue("LightManager","PointLight" + std::to_string(num),"color",color);
	variables->addValue("LightManager","PointLight" + std::to_string(num),"intensity",intensity);
	variables->addValue("LightManager","PointLight" + std::to_string(num),"radius",radius);
	variables->addValue("LightManager","PointLight" + std::to_string(num),"decay",decay);
}

void PointLight::DebugUpdate(){
#ifdef _DEBUG/*
	ImGui::Begin("PointLight");
	ImGui::DragFloat3("Position",&this->pos.x,0.01f);
	ImGui::ColorEdit3("Color",&this->color.x);
	ImGui::SliderFloat("Intensity",&this->intensity,0.0f,1.0f);
	ImGui::DragFloat("Radius",&this->radius,0.1f,0.1f,FLT_MAX);
	ImGui::DragFloat("Decay",&this->decay,0.1f,0.1f,FLT_MAX);
	ImGui::End();*/
#endif // _DEBUG
}
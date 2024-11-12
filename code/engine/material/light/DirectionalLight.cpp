#include "DirectionalLight.h"

#include "globalVariables/GlobalVariables.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

void DirectionalLight::Init(int32_t num){
	GlobalVariables* variables = GlobalVariables::getInstance();

	variables->addValue("LightManager","DirectionalLight" + std::to_string(num),"color",color);
	variables->addValue("LightManager","DirectionalLight" + std::to_string(num),"direction",direction);
	variables->addValue("LightManager","DirectionalLight" + std::to_string(num),"intensity",intensity);
}

void DirectionalLight::DebugUpdate(){
#ifdef _DEBUG
	this->direction = this->direction.Normalize();
#endif // _DEBUG
}
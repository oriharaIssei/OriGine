#include "material/light/SpotLight.h"

#include "globalVariables/GlobalVariables.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

void SpotLight::Init(int32_t num){
	GlobalVariables* variables = GlobalVariables::getInstance();

	variables->addValue("LightManager","SpotLight" + std::to_string(num),"pos",pos);
	variables->addValue("LightManager","SpotLight" + std::to_string(num),"color",color);
	variables->addValue("LightManager","SpotLight" + std::to_string(num),"intensity",intensity);
	variables->addValue("LightManager","SpotLight" + std::to_string(num),"direction",direction);
	variables->addValue("LightManager","SpotLight" + std::to_string(num),"distance",distance);
	variables->addValue("LightManager","SpotLight" + std::to_string(num),"decay",decay);
	variables->addValue("LightManager","SpotLight" + std::to_string(num),"cosAngle",cosAngle);
	variables->addValue("LightManager","SpotLight" + std::to_string(num),"cosFalloffStart",cosFalloffStart);
}


void SpotLight::DebugUpdate(){
#ifdef _DEBUG
	this->direction = direction.Normalize();
#endif // _DEBUG
}
#include "material/light/DirectionalLight.h"

#include "globalVariables/GlobalVariables.h"

void DirectionalLight::Init(const std::string& scene,int32_t index){
	GlobalVariables* variables = GlobalVariables::getInstance();

	variables->addValue(scene,"DirectionalLight" + std::to_string(index),"color",color);
	variables->addValue(scene,"DirectionalLight" + std::to_string(index),"direction",direction);
	variables->addValue(scene,"DirectionalLight" + std::to_string(index),"intensity",intensity);
}

#include "material/light/PointLight.h"

#include "globalVariables/GlobalVariables.h"

void PointLight::Init(const std::string& scene,int32_t index){
	GlobalVariables* variables = GlobalVariables::getInstance();

	variables->addValue(scene,"PointLight" + std::to_string(index),"pos",pos);
	variables->addValue(scene,"PointLight" + std::to_string(index),"color",color);
	variables->addValue(scene,"PointLight" + std::to_string(index),"intensity",intensity);
	variables->addValue(scene,"PointLight" + std::to_string(index),"radius",radius);
	variables->addValue(scene,"PointLight" + std::to_string(index),"decay",decay);
}
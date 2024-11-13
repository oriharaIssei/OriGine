#include "material/light/SpotLight.h"

#include "globalVariables/GlobalVariables.h"

void SpotLight::Init(const std::string& scene,int32_t index){
	GlobalVariables* variables = GlobalVariables::getInstance();

	variables->addValue(scene,"SpotLight" + std::to_string(index),"pos",pos);
	variables->addValue(scene,"SpotLight" + std::to_string(index),"color",color);
	variables->addValue(scene,"SpotLight" + std::to_string(index),"intensity",intensity);
	variables->addValue(scene,"SpotLight" + std::to_string(index),"direction",direction);
	variables->addValue(scene,"SpotLight" + std::to_string(index),"distance",distance);
	variables->addValue(scene,"SpotLight" + std::to_string(index),"decay",decay);
	variables->addValue(scene,"SpotLight" + std::to_string(index),"cosAngle",cosAngle);
	variables->addValue(scene,"SpotLight" + std::to_string(index),"cosFalloffStart",cosFalloffStart);
}

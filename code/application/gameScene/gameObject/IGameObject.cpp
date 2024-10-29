#include "IGameObject.h"

#include "imgui/imgui.h"

#include "System.h"

std::list<std::pair<std::string,std::string>> IGameObject::textureList_;

void IGameObject::Init([[maybe_unused]] const std::string& directoryPath,const std::string& objectName){
	name_ = objectName;
	materialManager_ = System::getInstance()->getMaterialManager();

	checkedMaterial_.push_back(0);
}

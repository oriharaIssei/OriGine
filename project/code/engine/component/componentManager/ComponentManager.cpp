#include "ComponentInstanceManager.h"

// リフレクション登録のためのマクロ
#define SetReflection(className) namespace className##Reflection

ComponentInstanceManager* ComponentInstanceManager::getInstance(){
	static ComponentInstanceManager instance{};
	return &instance;
}
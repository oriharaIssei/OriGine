#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include <stdint.h>
#include <string>

#include "../IComponent.h"
#include "module/IModule.h"

/// <summary>
/// 生存している Component を 制御 する 
/// </summary>
class ComponentManager
	:public IModule{
public:
	static ComponentManager* getInstance();


	bool PutOn(IComponent* instance,
			   const std::string& variableName);
	bool Destroy(const std::string& variableName);

	IComponent* GetVariable(const std::string& name){
		return activeInstancesPool_[activeInstanceIndices_[name]].get();
	}
private:
	//  情報 を 保存 するところ
	std::vector<std::unique_ptr<IComponent>> activeInstancesPool_;
	// 文字列で activeInstancePool_ から オブジェクトを 拾うためのもの
	std::unordered_map<std::string,size_t> activeInstanceIndices_;
	// activeInstancePool_ にて 使われていない Index 
	std::vector<size_t> freeIndices_;
public:
	std::string getTypeName(IComponent* instance)const;
};
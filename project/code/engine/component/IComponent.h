#pragma once

#include <stdint.h>
#include <string>

///===================================================
/// asset などの データを持ち， 
/// 複数の機能が まとまった クラス
/// Module と違い， Engine 内に 1つなどの制約はない
///===================================================
class IComponent{
	friend class ComponentManager;
public:
	IComponent(){}
	~IComponent(){}

	virtual void Init() = 0;
	virtual void Update() = 0;

	void Destroy();
private:
	std::string typeName_;
	std::string variableName_;
public:
	std::string getTypeName() const{
		return typeName_;
	}
	std::string getVariableName() const{
		return typeName_;
	}
};

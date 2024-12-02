#pragma once

#include <stdint.h>
#include <string>

///===================================================
/// asset などの データを持ち， 
/// 複数の機能が まとまった クラス
/// Module と違い， Engine 内に 1つなどの制約はない
///===================================================
class IComponent{
public:
	IComponent();
	virtual ~IComponent(){}

	virtual void Init();
	virtual void Update();

private:
	std::string typeName_;
public:
	const std::string& getTypeName()const{ return typeName_; }
};
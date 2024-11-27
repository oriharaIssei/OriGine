#pragma once

///===================================================
/// asset などの データを持ち， 
/// 複数の機能が まとまった クラス
/// Module と違い， Engine 内に 1つなどの制約はない
///===================================================
class IComponent{
public:
	IComponent(){}
	virtual ~IComponent(){}
};
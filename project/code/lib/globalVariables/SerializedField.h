#pragma once

#include "GlobalVariables.h"

template <typename T>
class SerializedField{
public:
	// コンストラクタ
	SerializedField(const std::string& scene,const std::string& group,const std::string& value){
		value_ = GlobalVariables::getInstance()->addValue<T>(scene,group,value);
	}

	// デストラクタ
	~SerializedField(){}

	// 値を読み取る
	const T* operator->() const{ return value_; }

	// 値を取得（暗黙変換用）
	operator T() const{
		if(value_){
			return *value_;
		}
		throw std::runtime_error("Attempted to access uninitialized value.");
	}
	operator const T* () const{
		if(value_){
			return value_;
		}
		throw std::runtime_error("Attempted to access uninitialized value.");
	}

	// 値を設定する
	void setValue(const T& newValue){
		if(value_){
			*value_ = newValue;
		}
	}

private:
	T* value_;
};
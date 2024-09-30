#pragma once

#include <cmath>

#include "Vector2.h"
#include "Vector3.h"

struct Vector4{
	float x;
	float y;
	float z;
	float w;

	Vector4(const float &x,const float &y,const float &z,const float &w)
		: x(x),y(y),z(z),w(w){
	}

	Vector4(const Vector2 &xy,const Vector2 &zw)
		: x(xy.x),y(xy.y),z(zw.x),w(zw.y){
	}

	Vector4(const float &x,const Vector2 &yz,const float &w)
		: x(x),y(yz.x),z(yz.y),w(w){
	}

	Vector4(const Vector3 &xyz,const float &w)
		: x(xyz.x),y(xyz.y),z(xyz.z),w(w){
	}

	Vector4(const float &x,const Vector3 &yzw)
		: x(x),y(yzw.x),z(yzw.y),w(yzw.z){
	}

	Vector4(const Vector2 &xy,const float &z,const float &w)
		: x(xy.x),y(xy.y),z(z),w(w){
	}

	Vector4(const float &x,const float &y,const Vector2 &zw)
		: x(x),y(y),z(zw.x),w(zw.y){
	}

	Vector4() :x(0),y(0),z(0){};

	// 加算演算子のオーバーロード
	Vector4 operator+(const Vector4 &other){
		return {x + other.x,y + other.y,z + other.z,w + other.w};
	}

	// 減算演算子のオーバーロード
	Vector4 operator-(const Vector4 &other){
		return {x - other.x,y - other.y,z - other.z,w - other.w};
	}

	// 乗算演算子のオーバーロード
	Vector4 operator*(float scalar){
		return {x * scalar,y * scalar,z * scalar,w * scalar};
	}

	// 比較演算子のオーバーロード (==)
	bool operator==(const Vector4 &v)const{
		return x == v.x && y == v.y && z == v.z && w == v.w;
	}

	Vector4 cross(const Vector4 &other){
		return {
			y * other.z - z * other.y,
			z * other.x - x * other.z,
			x * other.y - y * other.x,
			0.0f
		};
	}

	// dot積を計算する関数
	float dot(const Vector4 &other){
		return x * other.x + y * other.y + z * other.z + w * other.w;
	}

	// 長さを計算する関数
	float length(){
		return std::sqrt(x * x + y * y + z * z + w * w);
	}
};
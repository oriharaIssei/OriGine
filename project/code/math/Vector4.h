#pragma once

#include <cmath>

#include <Vector2.h>
#include <Vector3.h>

struct Vector4 final{
	float x,y,z,w;

	// コンストラクタ
	Vector4(float xValue,float yValue,float zValue,float wValue)
		: x(xValue),y(yValue),z(zValue),w(wValue){}
	Vector4(int X,int Y,int Z,int W)
		: x((float)X),y((float)Y),z((float)Z),w((float)W){}
	Vector4(const float* x_ptr,const float* y_ptr,const float* z_ptr,const float* w_ptr)
		: x(*x_ptr),y(*y_ptr),z(*z_ptr),w(*w_ptr){}
	Vector4(const float* ptr)
		: x(ptr[0]),y(ptr[1]),z(ptr[2]),w(ptr[3]){}
	Vector4(const Vector2& xy,const Vector2& zw)
		: x(xy.x),y(xy.y),z(zw.x),w(zw.y){}
	Vector4(float x,const Vector2& yz,float w)
		: x(x),y(yz.x),z(yz.y),w(w){}
	Vector4(const Vector3& xyz,float w)
		: x(xyz.x),y(xyz.y),z(xyz.z),w(w){}
	Vector4(const Vector2& xy,float z,float w)
		: x(xy.x),y(xy.y),z(z),w(w){}
	Vector4(float x,float y,const Vector2& zw)
		: x(x),y(y),z(zw.x),w(zw.y){}
	Vector4(float x,const Vector3& yzw)
		: x(x),y(yzw.x),z(yzw.y),w(yzw.z){}
	Vector4():x(0),y(0),z(0),w(0){}


	// ベクトルの長さ
	float length() const{ return sqrtf(x * x + y * y + z * z + w * w); }
	static float Length(const Vector4& v){ return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w); }

	// ベクトルの長さの二乗
	float lengthSq() const{ return x * x + y * y + z * z + w * w; }
	static float LengthSq(const Vector4& v){ return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w; }

	// 内積
	float dot(const Vector4& another) const{
		return (x * another.x) + (y * another.y) + (z * another.z) + (w * another.w);
	}
	static float Dot(const Vector4& v,const Vector4& another){
		return (v.x * another.x) + (v.y * another.y) + (v.z * another.z) + (v.w * another.w);
	}

	// 正規化
	Vector4 normalize() const{
		float len = length();
		if(len == 0) return *this;
		return (*this / len);
	}
	static Vector4 Normalize(const Vector4& v){
		float len = v.length();
		if(len == 0) return v;
		return (v / len);
	}

	// 演算子オーバーロード (+, -, *, /, 等)
	Vector4 operator+(const Vector4& other) const{
		return Vector4(x + other.x,y + other.y,z + other.z,w + other.w);
	}
	void operator+=(const Vector4& other){
		x += other.x;
		y += other.y;
		z += other.z;
		w += other.w;
	}

	Vector4 operator-(const Vector4& other) const{
		return Vector4(x - other.x,y - other.y,z - other.z,w - other.w);
	}
	Vector4 operator-() const{ return Vector4(-x,-y,-z,-w); }
	void operator-=(const Vector4& other){
		x -= other.x;
		y -= other.y;
		z -= other.z;
		w -= other.w;
	}

	Vector4 operator*(float scalar) const{
		return Vector4(x * scalar,y * scalar,z * scalar,w * scalar);
	}
	void operator*=(float scalar){
		x *= scalar;
		y *= scalar;
		z *= scalar;
		w *= scalar;
	}

	Vector4 operator/(float scalar) const{
		if(scalar != 0){
			return Vector4(x / scalar,y / scalar,z / scalar,w / scalar);
		} else{
			return Vector4(0.0f,0.0f,0.0f,0.0f);
		}
	}
	void operator/=(float scalar){
		if(scalar != 0){
			x /= scalar;
			y /= scalar;
			z /= scalar;
			w /= scalar;
		} else{
			x = 0;
			y = 0;
			z = 0;
			w = 0;
		}
	}

	// 等号演算子のオーバーロード
	bool operator==(const Vector4& other) const{
		return (x == other.x && y == other.y && z == other.z && w == other.w);
	}
	bool operator!=(const Vector4& other) const{
		return !(*this == other);
	}
};

inline Vector4 operator*(float scalar,const Vector4& vec){
	return Vector4(vec.x * scalar,vec.y * scalar,vec.z * scalar,vec.w * scalar);
}

inline Vector4 Lerp(const Vector4& start,const Vector4& end,float time){
	return {
		std::lerp(start.x,end.x,time),
		std::lerp(start.y,end.y,time),
		std::lerp(start.z,end.z,time),
		std::lerp(start.w,end.w,time)
	};
}

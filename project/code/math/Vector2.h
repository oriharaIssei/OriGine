#pragma once

#include <cmath>

//=====================================
// x&yをもつ単位
//=====================================

struct Vector2 final{
	float x;
	float y;

	// コンストラクタ
	Vector2(float xValue,float yValue): x(xValue),y(yValue){}
	Vector2(int X,int Y):x((float)X),y((float)Y){}
	Vector2(const float* x_ptr,const float* y_ptr): x(*x_ptr),y(*y_ptr){}
	Vector2(const float* ptr): x(ptr[0]),y(ptr[1]){}
	Vector2(): x(0.0f),y(0.0f){}

	// ベクトルの長さを計算
	float length()const{ return sqrtf(x * x + y * y); }
	static float Length(const Vector2& v){ return sqrtf(v.x * v.x + v.y * v.y); }

	float lengthSq()const{ return (this->x * this->x + this->y * this->y); }
	static float LengthSq(const Vector2& v1){ return (v1.x * v1.x + v1.y * v1.y); }

	float dot()const{ return x * x + y * y; }
	float dot(const Vector2& another) const{ return (this->x * another.x) + (this->y * another.y); }

	static float Dot(const Vector2& v){ return v.x * v.x + v.y * v.y; }
	static float Dot(const Vector2& v,const Vector2& another){ return (v.x * another.x) + (v.y * another.y); }

	float cross(const Vector2& another)const{ return (this->x * another.y) - (this->y * another.x); }
	static float Cross(const Vector2& v,const Vector2& another){ return (v.x * another.y) - (v.y * another.x); }

	Vector2 normalize()const{
		float length = this->length();
		if(length == 0){
			return *this;
		}
		Vector2 result = *this;
		return (result / length);
	}

	static Vector2 Normalize(const Vector2& v){
		float length = v.length();
		if(length == 0){
			return v;
		}
		Vector2 result = v;
		return (result / length);
	}

	Vector2 operator+(const Vector2& other) const{
		return Vector2(x + other.x,y + other.y);
	}
	void operator+=(const Vector2& other){
		x += other.x;
		y += other.y;
	}

	Vector2 operator-(const Vector2& other) const{
		return Vector2(x - other.x,y - other.y);
	}
	Vector2 operator-()const{ return Vector2(-x,-y); }
	void operator-=(const Vector2& other){
		x -= other.x;
		y -= other.y;
	}

	Vector2 operator*(const float& scalar) const{
		return Vector2(x * scalar,y * scalar);
	}
	void operator*=(const float& scalar){
		x *= scalar;
		y *= scalar;
	}

	Vector2 operator/(const float& scalar) const{
		if(scalar != 0){
			return Vector2(x / scalar,y / scalar);
		} else{
			// ゼロで割る場合の処理を追加
			return Vector2(0.0f,0.0f);
		}
	}
	void operator/=(const float& scalar){
		if(scalar != 0){
			x /= scalar;
			y /= scalar;
		} else{
			// ゼロで割る場合の処理を追加
			x = 0;
			y = 0;
		}
	}

	// 等号演算子のオーバーロード
	bool operator==(const Vector2& other) const{
		return (x == other.x && y == other.y);
	}

	// 不等号演算子のオーバーロード
	bool operator!=(const Vector2& other) const{
		return !(*this == other);
	}
};

inline Vector2 operator*(const float& scalar,const Vector2& vec){
	return Vector2(vec.x * scalar,vec.y * scalar);
}

inline Vector2 Lerp(const Vector2& start,const Vector2& end,float time){
	return {
		std::lerp(start.x,end.x,time),
		std::lerp(start.y,end.y,time)
	};
}
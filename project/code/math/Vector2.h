#pragma once
#define _USE_MATH_DEFINES
#include <math.h>

//=====================================
// x&yをもつ単位
//=====================================

struct Vector2 final {
	float x;
	float y;

	// コンストラクタ
	Vector2(float xValue, float yValue): x(xValue), y(yValue) {}
	Vector2(int X, int Y):x((float)X), y((float)Y) {};
	Vector2(float *x_ptr, float *y_ptr): x(*x_ptr), y(*y_ptr) {}
	Vector2(): x(0.0f), y(0.0f) {}

	// ベクトルの長さを計算
	float Length()const;
	float Length(const Vector2 &another);

	float LengthSq()const;

	float Dot()const;
	float Dot(Vector2 another);

	float Cross(const Vector2 &another);

	Vector2 operator+(const Vector2 &other) const {
		return Vector2(x + other.x, y + other.y);
	}
	void operator+=(const Vector2 &other) {
		x += other.x;
		y += other.y;
	}

	Vector2 operator-(const Vector2 &other) const {
		return Vector2(x - other.x, y - other.y);
	}
	Vector2 operator-()const;
	void operator-=(const Vector2 &other) {
		x -= other.x;
		y -= other.y;
	}

	Vector2 operator*(const float &scalar) const {
		return Vector2(x * scalar, y * scalar);
	}
	void operator*=(const float &scalar) {
		x *= scalar;
		y *= scalar;
	}

	Vector2 operator/(const float &scalar) const {
		if(scalar != 0) {
			return Vector2(x / scalar, y / scalar);
		} else {
			// ゼロで割る場合の処理を追加
			return Vector2(0.0f, 0.0f);
		}
	}
	void operator/=(const float &scalar) {
		if(scalar != 0) {
			x /= scalar;
			y /= scalar;
		} else {
			// ゼロで割る場合の処理を追加
			x = 0;
			y = 0;
		}
	}

	// 等号演算子のオーバーロード
	bool operator==(const Vector2 &other) const {
		return (x == other.x && y == other.y);
	}

	// 不等号演算子のオーバーロード
	bool operator!=(const Vector2 &other) const {
		return !(*this == other);
	}
};

inline Vector2 Normalize(const Vector2 &a) {
	float length = a.Length();
	if(length == 0) {
		return a;
	}
	Vector2 result = a;
	return (result / length);
};

inline float Vector2::Length() const {
	return sqrtf(x * x + y * y);
}

inline float Vector2::Length(const Vector2 &another) {
	return sqrtf(this->x * another.x + this->y * another.y);
}

inline float Vector2::LengthSq() const {
	return (this->x * this->x + this->y * this->y);
}

inline float Vector2::Dot() const {
	return x * x + y * y;
}

inline float Vector2::Dot(Vector2 another) {
	return (this->x * another.x) + (this->y * another.y);
}

inline float Vector2::Cross(const Vector2 &another) {
	return (this->x * another.y) - (this->y * another.x);
}

inline Vector2 Vector2::operator-() const {
	return Vector2(-x, -y);
}
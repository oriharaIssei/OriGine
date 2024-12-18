#include "Quaternion.h"

#include <cmath>
#include <iomanip>
#include <iostream>

#pragma region"Operators"
Quaternion Quaternion::operator+(const Quaternion& q) const{
	return {
		x + q.x,
		y + q.y,
		z + q.z,
		w + q.w,
	};
}

Quaternion Quaternion::operator-(const Quaternion& q) const{
	return {
		x - q.x,
		y - q.y,
		z - q.z,
		w - q.w,
	};
}

Quaternion Quaternion::operator*(const Quaternion& q)const{
	return {
		w * q.x + x * q.w + y * q.z - z * q.y, // x'
		w * q.y + y * q.w + z * q.x - x * q.z, // y'
		w * q.z + z * q.w + x * q.y - y * q.x, // z'
		w * q.w - x * q.x - y * q.y - z * q.z  // w'
	};
}

Quaternion Quaternion::operator*(float scalar) const{
	return Quaternion(
		x * scalar,
		y * scalar,
		z * scalar,
		w * scalar
	);
}

Quaternion Quaternion::operator/(float scalar)const{
	return Quaternion(
		this->x / scalar,
		this->y / scalar,
		this->z / scalar,
		this->w / scalar
	);
}

Quaternion* Quaternion::operator=(const Quaternion& q){
	this->x = q.x;
	this->y = q.y;
	this->z = q.z;
	this->w = q.w;
	return this;
}

Quaternion* Quaternion::operator+=(const Quaternion& q){
	this->x += q.x;
	this->y += q.y;
	this->z += q.z;
	this->w += q.w;
	return this;
}

Quaternion* Quaternion::operator*=(const Quaternion& q){
	this->x *= q.x;
	this->y *= q.y;
	this->z *= q.z;
	this->w *= q.w;
	return this;
}

Quaternion* Quaternion::operator*=(float scalar){
	this->x *= scalar;
	this->y *= scalar;
	this->z *= scalar;
	this->w *= scalar;
	return this;
}

Quaternion* Quaternion::operator/=(float scalar){
	this->x /= scalar;
	this->y /= scalar;
	this->z /= scalar;
	this->w /= scalar;
	return this;
}

Quaternion operator*(float scalar,const Quaternion& q){
	return {
		q.x * scalar,
		q.y * scalar,
		q.z * scalar,
		q.w * scalar
	};
}

Quaternion operator/(float scalar,const Quaternion& q){
	return {
		q.x / scalar,
		q.y / scalar,
		q.z / scalar,
		q.w / scalar
	};
}
#pragma endregion

Quaternion Quaternion::Inverse(const Quaternion& q){
	float normSq = q.normSq();// ノルムの二乗
	if(normSq == 0.0f){
		// ノルムが0の場合、逆元は存在しない
		return {0.0f,0.0f,0.0f,0.0f}; // エラー処理としてゼロ四元数を返す
	}
	Quaternion conjugate = q.Conjugation();
	return conjugate / normSq; // 共役をノルムの二乗で割る
}

Quaternion Quaternion::inverse() const{
	float normSq = this->normSq();// ノルムの二乗
	if(normSq == 0.0f){
		// ノルムが0の場合、逆元は存在しない
		return {0.0f,0.0f,0.0f,0.0f}; // エラー処理としてゼロ四元数を返す
	}
	Quaternion conjugate = this->Conjugation();
	return conjugate / normSq; // 共役をノルムの二乗で割る
}

float Quaternion::Norm(const Quaternion& q){
	return sqrtf(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
}

float Quaternion::norm() const{
	return sqrtf(this->w * this->w + this->x * this->x + this->y * this->y + this->z * this->z);
}

float Quaternion::NormSq(const Quaternion& q){
	return q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z;
}

float Quaternion::normSq() const{
	return this->w * this->w + this->x * this->x + this->y * this->y + this->z * this->z;
}

Quaternion Quaternion::Normalize(const Quaternion& q){
	float norm = q.norm();
	if(norm == 0.0f){
		return {0.0f,0.0f,0.0f,0.0f};
	}
	return q / norm;
}

Quaternion Quaternion::normalize() const{
	float norm = this->norm();
	if(norm == 0.0f){
		return {0.0f,0.0f,0.0f,0.0f};
	}
	return *this / norm;
}

float Quaternion::Dot(const Quaternion& q0,const Quaternion& q1){
	return q0.x * q1.x + q0.y * q1.y + q0.z * q1.z + q0.w * q1.w;
}

float Quaternion::dot(const Quaternion& q)const{
	return x * q.x + y * q.y + z * q.z + w * q.w;
}

Quaternion Quaternion::RotateAxisAngle(const Vector3& axis,float angle){
	float halfAngle = angle / 2.0f;
	return Quaternion(
		axis * sinf(halfAngle),
		cosf(halfAngle)
	);
}

void Quaternion::Show(){
	std::cout << std::fixed << std::setprecision(3) << x << " ";
	std::cout << std::fixed << std::setprecision(3) << y << " ";
	std::cout << std::fixed << std::setprecision(3) << z << " ";
	std::cout << std::fixed << std::setprecision(3) << w << " ";
	std::cout << "\n" << std::endl;
}

Quaternion Slerp(const Quaternion& q0,const Quaternion& q1,float t){
	float dot = q0.dot(q1);
	Quaternion rotate = q0;

	/// dot が 0未満なら 反転(逆回転)
	if(dot < 0){
		rotate = -rotate;
		dot = -dot;
	}

	float theta = acosf(dot);
	float sinTheta = sinf(theta);

	float scale0 = sinf((1.0f - t) * theta) / sinTheta;
	float scale1 = sinf(t * theta) / sinTheta;

	return q0 * scale0 + q1 * scale1;
}
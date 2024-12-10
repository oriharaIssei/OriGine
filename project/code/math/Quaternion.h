#pragma once

#include "Vector3.h"

/// <summary>
/// 四元数
/// </summary>
struct Quaternion{
	Quaternion(){}
	Quaternion(float _x,float _y,float _z,float _w):x(_x),y(_y),z(_z),w(_w){}
	Quaternion(const Vector3& v,float _w):x(v.x),y(v.y),z(v.z),w(_w){}

	float x = 0;
	float y = 0;
	float z = 0;
	float w = 0;
	Quaternion operator+(const Quaternion& q)const;
	Quaternion operator-(const Quaternion& q)const;
	Quaternion operator-()const{
		return {
			-x,
			-y,
			-z,
			-w
		};
	}
	Quaternion operator*(const Quaternion& q)const;
	Quaternion operator*(float scalar)const;
	Quaternion operator/(float scalar)const;

	Quaternion* operator=(const Quaternion& q);
	Quaternion* operator+=(const Quaternion& q);
	Quaternion* operator*=(const Quaternion& q);
	Quaternion* operator*=(float scalar);
	Quaternion* operator/=(float scalar);

	static Quaternion Identity(){
		return Quaternion(
			0.0f,
			0.0f,
			0.0f,
			1.0f
		);
	}
	static Quaternion Inverse(const Quaternion& q);
	Quaternion inverse()const;

	static Quaternion Conjugation(const Quaternion& q){
		return Quaternion(
			-q.x,
			-q.y,
			-q.z,
			q.w
		);
	}
	Quaternion Conjugation()const{
		return Quaternion(
			-this->x,
			-this->y,
			-this->z,
			this->w
		);
	}

	static float Norm(Quaternion q);
	float norm() const;

	static float NormSq(Quaternion q);
	float normSq() const;

	static Quaternion Normalize(Quaternion q);
	Quaternion normalize() const;

	static float Dot(const Quaternion& q0,const Quaternion& q1);
	float dot(const Quaternion& q)const;

	static Quaternion RotateAxisAngle(const Vector3& axis,float angle);

	void Show();
};

Quaternion operator*(float scalar,const Quaternion& q);
Quaternion operator/(float scalar,const Quaternion& q);

Quaternion Slerp(const Quaternion& q0,const Quaternion& q1,float t);
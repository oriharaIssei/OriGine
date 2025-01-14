#include "Vector3.h"

#include "Quaternion.h"

Vector3 Vector3::RotateVector(const Quaternion& q) const{
	Quaternion r = Quaternion(*this,0.0f);
	r = q * r * q.Conjugation();
	return Vector3(r.x,r.y,r.z);
}

Vector3 Vector3::RotateVector(const Vector3& v,const Quaternion& q){
	Quaternion r = Quaternion(v,0.0f);
	r = q * r * q.Conjugation();
	return Vector3(r.x,r.y,r.z);
}
#include "SLerp.h"

#include "Easing.h"
#include "Vector3.h"

Vector3 Slerp(const float& t,const Vector3& start,const Vector3& end){
	// cosθを計算
	float dot = start.dot(end);

	if(dot > 0.9995 || dot < -0.9995){
		return Lerp(t,start,end).Normalize();
	}

	// θを計算し、sinθで割ることで補間を正規化
	float slerpedTheta = acos(dot) * t;
	Vector3 relativeVec = (end - start * dot).Normalize();

	return start * std::cosf(slerpedTheta) + relativeVec * sinf(slerpedTheta);
}
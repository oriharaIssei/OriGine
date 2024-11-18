#include "Vector4.h"

#include "Matrix4x4.h"

Vector4 operator*(const Matrix4x4& mat,const Vector4& vec){
	float result[4] = {0.0f,0.0f,0.0f,0.0f};
	float hcs[4] = {vec.x,vec.y,vec.z,1.0f};

	for(int r = 0; r < 4; r++){
		for(int c = 0; c < 4; c++){
			result[r] += hcs[c] * mat[c][r];
		}
	}

	return Vector4(result[0],result[1],result[2],result[3]);
}

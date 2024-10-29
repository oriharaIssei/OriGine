#include "Matrix4x4.h"

#include <cmath>

#include "assert.h"

Matrix4x4 Matrix4x4::operator+(const Matrix4x4 &another) const {
	Matrix4x4 result;
	for(int row = 0; row < 4; row++) {
		for(int col = 0; col < 4; col++) {
			result.m[row][col] = this->m[row][col] + another.m[row][col];
		}
	}
	return result;
}

Matrix4x4 Matrix4x4::operator-(const Matrix4x4 &another) const {
	Matrix4x4 result;
	for(int row = 0; row < 4; row++) {
		for(int col = 0; col < 4; col++) {
			result.m[row][col] = this->m[row][col] - another.m[row][col];
		}
	}
	return result;
}

Matrix4x4 Matrix4x4::operator*(const Matrix4x4 &another) const {
	Matrix4x4 result;
	float sum = 0.0f;

	for(int row = 0; row < 4; ++row) {
		for(int col = 0; col < 4; ++col) {
			sum = 0.0f;
			for(int k = 0; k < 4; ++k) {
				sum += this->m[row][k] * another.m[k][col];
			}
			result[row][col] = sum;
		}
	}
	return result;
}

Matrix4x4 Matrix4x4::operator*(const float &scalar) const {
	Matrix4x4 result;
	for(int row = 0; row < 4; row++) {
		for(int col = 0; col < 4; col++) {
			result.m[row][col] = this->m[row][col] * scalar;
		}
	}
	return result;
}

Matrix4x4 *Matrix4x4::operator*=(const Matrix4x4 &another) {
	for(int row = 0; row < 4; ++row) {
		for(int col = 0; col < 4; ++col) {
			for(int k = 0; k < 4; ++k) {
				this->m[row][col] += this->m[row][k] * another.m[k][col];
			}
		}
	}
	return this;
}

Matrix4x4 Matrix4x4::Transpose()const {
	Matrix4x4 result;
	for(int row = 0; row < 4; row++) {
		for(int col = 0; col < 4; col++) {
			result.m[row][col] = this->m[col][row];
		}
	}
	return result;
}

Matrix4x4 Matrix4x4::Inverse() const {
	DirectX::XMMATRIX thisMat = MatrixToXMMATRIX();
	DirectX::XMVECTOR det = DirectX::XMMatrixDeterminant(thisMat);
	Matrix4x4 inverse;
	inverse.XMMATRIXToMatrix(DirectX::XMMatrixInverse(&det,thisMat));
	return  inverse;
}
Matrix4x4 MakeMatrix::Identity() { return Matrix4x4({1.0f,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f,0.0f,0.0f,0.0f,1.0f}); }

Matrix4x4 MakeMatrix::Translate(const Vector3 &vec) { return Matrix4x4({1.0f,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f,vec.x,vec.y,vec.z,1.0f}); }

Matrix4x4 MakeMatrix::Scale(const Vector3 &vec) {
	return Matrix4x4(
		{vec.x,0.0f,0.0f,0.0f,
		0.0f,vec.y,0.0f,0.0f,
		0.0f,0.0f,vec.z,0.0f,
		0.0f,0.0f,0.0f,1.0f});
}

Matrix4x4 MakeMatrix::RotateX(const float &radian) {
	return Matrix4x4({01.0f,.0f,0.0f,0.0f,0.0f,std::cosf(radian),std::sinf(radian),0.0f,0.0f,-std::sinf(radian),std::cosf(radian),0.0f,0.0f,0.0f,0.0f,1.0f});
}

Matrix4x4 MakeMatrix::RotateY(const float &radian) {
	return Matrix4x4({std::cosf(radian),0.0f,-std::sinf(radian),0.0f,0.0f,1.0f,0.0f,0.0f,std::sinf(radian),0.0f,std::cosf(radian),0.0f,0.0f,0.0f,0.0f,1.0f});
}

Matrix4x4 MakeMatrix::RotateZ(const float &radian) {
	return Matrix4x4({std::cosf(radian),std::sinf(radian),0.0f,0.0f,-std::sinf(radian),std::cosf(radian),0.0f,0.0f,0.0f,0.0f,1.0f,0.0f,0.0f,0.0f,0.0f,1.0f});
}

Matrix4x4 MakeMatrix::RotateXYZ(const Vector3 &radian) { return MakeMatrix::RotateZ(radian.z) * MakeMatrix::RotateX(radian.x) * MakeMatrix::RotateY(radian.y); }

Matrix4x4 MakeMatrix::RotateXYZ(const Matrix4x4 &x,const Matrix4x4 &y,const Matrix4x4 &z) { return z * x * y; }

Matrix4x4 MakeMatrix::Affine(const Vector3 &scale,const Vector3 &rotate,const Vector3 &translate) {
	return MakeMatrix::Scale(scale) * MakeMatrix::RotateXYZ(rotate) * MakeMatrix::Translate(translate);
}

Vector3 TransformNormal(const Vector3 &v,const Matrix4x4 &m) {
	// 平行移動を無視して計算
	Vector3 result = {
		v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0],
		v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1],
		v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2],
	};

	return result;
}

Matrix4x4 MakeMatrix::PerspectiveFov(const float &fovY,const float &aspectRatio,const float &nearClip,const float &farClip) {
	const float cot = 1.0f / std::tanf(fovY / 2.0f);
	return Matrix4x4(
		{(1.0f / aspectRatio) * cot,0.0f,0.0f,0.0f,0.0f,cot,0.0f,0.0f,0.0f,0.0f,farClip / (farClip - nearClip),1.0f,0.0f,0.0f,(-nearClip * farClip) / (farClip - nearClip),0.0f});
}

Matrix4x4 MakeMatrix::Orthographic(const float &left,const float &top,const float &right,const float &bottom,const float &nearClip,const float &farClip) {
	return Matrix4x4(
		{2.0f / (right - left),0.0f,0.0f,0.0f,0.0f,2.0f / (top - bottom),0.0f,0.0f,0.0f,0.0f,1.0f / (farClip - nearClip),0.0f,(left + right) / (left - right),
		(top + bottom) / (bottom - top),nearClip / (nearClip - farClip),1.0f});
}

Matrix4x4 MakeMatrix::ViewPort(const float &left,const float &top,const float &width,const float &height,const float &minDepth,const float &maxDepth) {
	return Matrix4x4({width / 2.0f,0.0f,0.0f,0.0f,0.0f,-(height / 2.0f),0.0f,0.0f,0.0f,0.0f,maxDepth - minDepth,0.0f,left + (width / 2.0f),top + (height / 2.0f),minDepth,1.0f});
}
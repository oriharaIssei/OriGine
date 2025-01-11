#pragma once

#include <Vector3.h>

#include <DirectXMath.h>

struct Quaternion;

struct Matrix4x4 {
	float m[4][4];

	///
	///	演算子
	///
	float *operator[](int index) { return m[index]; }
	const float *operator[](int index) const { return m[index]; }

	Matrix4x4 operator+(const Matrix4x4 &another) const;
	Matrix4x4 operator-(const Matrix4x4 &another) const;
	Matrix4x4 operator*(const Matrix4x4 &another) const;
	Matrix4x4 operator*(const float &scalar) const;

	Matrix4x4 *operator*=(const Matrix4x4 &another);

	Matrix4x4 transpose()const;
	static Matrix4x4 Transpose(const Matrix4x4& m);

	Matrix4x4 inverse() const;
	static Matrix4x4 Inverse(const Matrix4x4& m);
private:
	DirectX::XMMATRIX MatrixToXMMATRIX()const {
		return DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4 *>(this));
	}

	Matrix4x4 *XMMATRIXToMatrix(const DirectX::XMMATRIX &xmmat) {
		XMStoreFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4 *>(this),xmmat);
		return this;
	}
};

namespace MakeMatrix {
	Matrix4x4 Identity();

	Matrix4x4 Translate(const Vec3f &vec);
	Matrix4x4 Scale(const Vec3f &vec);

	Matrix4x4 RotateX(const float &radian);
	Matrix4x4 RotateY(const float &radian);
	Matrix4x4 RotateZ(const float &radian);
	Matrix4x4 RotateXYZ(const Vec3f &radian);
	Matrix4x4 RotateXYZ(const Matrix4x4 &x, const Matrix4x4 &y, const Matrix4x4 &z);

	Matrix4x4 RotateQuaternion(const Quaternion& q);

	Matrix4x4 RotateAxisAngle(const Vec3f& axis,float angle);

	Matrix4x4 Affine(const Vec3f &scale, const Vec3f &rotate, const Vec3f &translate);
	Matrix4x4 Affine(const Vec3f& scale,const Quaternion& rotate,const Vec3f& translate);

	Matrix4x4 PerspectiveFov(const float &fovY, const float &aspectRatio, const float &nearClip, const float &farClip);
	Matrix4x4 Orthographic(const float &left, const float &top, const float &right, const float &bottom, const float &nearClip, const float &farClip);
	Matrix4x4 ViewPort(const float &left, const float &top, const float &width, const float &height, const float &minDepth, const float &maxDepth);
} // namespace MakeMatrix

Vec3f TransformVector(const Vec3f &vec, const Matrix4x4 &matrix);
Vec3f TransformNormal(const Vec3f &v, const Matrix4x4 &m);

#pragma once

#include <wrl.h>

#include <d3d12.h>

#include <Matrix4x4.h>
#include <Transform.h>
#include <Vector3.h>

struct ConstBufferDataViewProjection{
	Vector3 cameraPos;
	float padding;
	Matrix4x4 view;       // ワールド → ビュー変換行列
	Matrix4x4 viewTranspose;
	Matrix4x4 projection; // ビュー → プロジェクション変換行列
};

class ViewProjection{
public:
	void Init();
	void Finalize();
	void UpdateMatrix();
	void ConvertToBuffer();
	void SetForRootParameter(ID3D12GraphicsCommandList *cmdList,UINT rootParameterNum)const;

	Vector3 rotate = {0.0f,0.0f,0.0f};
	Vector3 translate = {0.0f,0.0f,0.0f};

	Matrix4x4 viewMat;

	// 垂直方向視野角
	float fovAngleY = 45.0f * 3.141592654f / 180.0f;
	// ビューポートのアスペクト比
	float aspectRatio = (float)16 / 9;
	// 深度限界（手前側）
	float nearZ = 0.1f;
	// 深度限界（奥側）
	float farZ = 1000.0f;
	Matrix4x4 projectionMat;
private:
	Vector3 cameraPos_;
	ConstBufferDataViewProjection *mappingData_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> buff_;
};
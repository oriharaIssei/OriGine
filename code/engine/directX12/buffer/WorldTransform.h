#pragma once

#include <wrl.h>

#include <d3d12.h>

#include <array>

#include <functional>

#include <string>

#include <Matrix4x4.h>
#include <Transform.h>

struct ConstantBufferWorldMatrix {
	Matrix4x4 world;
};
class WorldTransform {
public:
	void Init();
	void Finalize();
	void Update();
	/// <summary>
	/// ImGuiでの要素表示関数(Matrixの更新はしない)
	/// </summary>
	void Debug(const std::string &transformName);
	void ConvertToBuffer();
	void SetForRootParameter(ID3D12GraphicsCommandList *cmdList,UINT rootParameterNum)const;

	Vector3 scale = {1.0f,1.0f,1.0f};
	Vector3 rotate;
	Vector3 translate;

	Matrix4x4 worldMat;

	WorldTransform *parent = nullptr;
private:
	ConstantBufferWorldMatrix *mappingWorldMat_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> buff_;
};
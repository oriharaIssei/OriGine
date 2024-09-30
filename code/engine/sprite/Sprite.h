#pragma once

#include "directX12/PipelineStateObj.h"
#include "directX12/shaderManager/ShaderManager.h"

#include <Matrix4x4.h>
#include <stdint.h>
#include <Vector2.h>
#include <Vector4.h>

#include <memory>
#include <wrl.h>

#include "directX12/dxCommand/DxCommand.h"

#include <string>

#include "directX12/buffer/Object3dMesh.h"

class Sprite{
public:
	static void Init();
	static void Finalize();
	static Sprite *Create(const Vector2 &pos,const Vector2 &size,const std::string &textureFilePath);
private:
	static BlendMode currentBlend_;
private:
	struct SpriteVertexData{
		Vector4 pos;
		Vector2 texcoord;
	};
	struct SpritConstBuffer{
		Vector4 color_;
		Matrix4x4 mat_;
		Matrix4x4 uvMat_;
	};
	struct SpriteMesh{
		void Init();
		SpriteVertexData *vertexData = nullptr;
		uint32_t *indexData = nullptr;

		Microsoft::WRL::ComPtr<ID3D12Resource> vertBuff = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> indexBuff = nullptr;

		D3D12_INDEX_BUFFER_VIEW ibView{};
		D3D12_VERTEX_BUFFER_VIEW vbView{};
	};

	static Matrix4x4 viewPortMat_;
	static void CreatePSO();
	static std::unique_ptr<DxCommand> dxCommand_;
	static std::array<PipelineStateObj *,kBlendNum> pso_;
public:
	void Draw();
public:
	Vector3 uvScale = {1.0f,1.0f,1.0f};
	Vector3 uvRotate,uvTranslate;

private:
	std::unique_ptr<SpriteMesh> meshBuff_;

	Matrix4x4 worldMat_;

	SpritConstBuffer *mappingConstBufferData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> constBuff_;

	uint32_t th_;
public:
	void setColor(const Vector4 &color){ mappingConstBufferData_->color_ = color; }
	void setSize(const Vector2 &size);
	void setPos(const Vector2 &pos);
	static void setBlendMode(BlendMode blend){ currentBlend_ = blend; }
};
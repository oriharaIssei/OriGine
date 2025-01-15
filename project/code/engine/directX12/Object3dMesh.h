#pragma once

#include "d3d12.h"

#include "directX12/DxResource.h"

#include <Vector2.h>
#include <Vector3.h>
#include <Vector4.h>

struct TextureVertexData {
	Vec4f pos;
	Vec2f texCoord;
	Vec3f normal;
	TextureVertexData *operator=(const TextureVertexData &vert) {
		this->pos = vert.pos;
		this->texCoord = vert.texCoord;
		this->normal = vert.normal;
		return this;
	}

	bool operator == (const TextureVertexData &vert) {
		if(this->pos != vert.pos) { return false; }
		if(this->texCoord != vert.texCoord) { return false; }
		if(this->normal != vert.normal) { return false; }
		return true;
	}
};
struct PrimitiveVertexData {
	Vec4f pos;
	Vec3f normal;
	PrimitiveVertexData(const TextureVertexData &vert) {
		this->pos = vert.pos;
		this->normal = vert.normal;
	}
	PrimitiveVertexData *operator=(const PrimitiveVertexData &vert) {
		this->pos = vert.pos;
		this->normal = vert.normal;
		return this;
	}
	PrimitiveVertexData *operator=(const TextureVertexData &vert) {
		this->pos = vert.pos;
		this->normal = vert.normal;
		return this;
	}
};

class IObject3dMesh {
public:
	virtual ~IObject3dMesh() {};
	void Finalize() { vertBuff.Finalize(); indexBuff.Finalize(); }
	/// <summary>
	/// VertexDataを設定後に実行
	/// </summary>
	/// <param name="vertexSize">総頂点数</param>
	virtual void Create(UINT vertexSize,UINT indexSize) = 0;
	uint32_t *indexData = nullptr;

	DxResource vertBuff;
	DxResource indexBuff;

	D3D12_INDEX_BUFFER_VIEW ibView {};
	D3D12_VERTEX_BUFFER_VIEW vbView {};
};

class TextureObject3dMesh :public IObject3dMesh {
public:
	TextureVertexData *vertData = nullptr;
	void Create(UINT vertexSize,UINT indexSize)override;
};

class PrimitiveObject3dMesh :public IObject3dMesh {
public:
	PrimitiveVertexData *vertData = nullptr;
	void Create(UINT vertexSize,UINT indexSize)override;
};
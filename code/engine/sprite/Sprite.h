#pragma once

#include "SpriteCommon.h"

#include "Vector2.h"

struct SpriteVertexData
{
	Vector4 pos;
	Vector2 texcoord;
};
struct SpritConstBuffer
{
	Vector4 color_;
	Matrix4x4 mat_;
	Matrix4x4 uvMat_;
};
struct SpriteMesh
{
	void Init();
	SpriteVertexData* vertexData = nullptr;
	uint32_t* indexData = nullptr;

	DxResource vertBuff;
	DxResource indexBuff;

	D3D12_INDEX_BUFFER_VIEW ibView{};
	D3D12_VERTEX_BUFFER_VIEW vbView{};
};

class SpriteCommon;
class Sprite{
	friend class SpriteCommon;
public:
	Sprite(SpriteCommon* spriteCommon):spriteCommon_(spriteCommon){}
	~Sprite() = default;

	/// <summary>
	/// SpriteCommon::Create と同じ
	/// </summary>
	/// <param name="filePath"></param>
	void Init(const std::string& filePath);
	void Draw();

	void UpdateMatrix();
private:
	SpriteCommon* spriteCommon_;

	Vector2 size_;
	float rotate_;
	Vector2 pos_;
	Matrix4x4 worldMat_;

	Vector3 uvScale_;
	Vector3 uvRotate_;
	Vector3 uvTranslate_;
	Matrix4x4 uvMat_;

	Vector4 color_;

	SpritConstBuffer *mappingConstBufferData_;
	std::unique_ptr<SpriteMesh> meshBuff_;
	DxResource constBuff_;

	uint32_t textureIndex_;
public:
	void SetSize(const Vector2& size) { size_ = size; }
	const Vector2 &GetSize() const { return size_; }
	
	void SetRotate(float rotate) { rotate_ = rotate; }
	float GetRotate() const { return rotate_; }
	
	void SetPosition(const Vector2& pos) { pos_ = pos; }
	const Vector2& GetPosition() const { return pos_; }

	void SetUVScale(const Vector3& uvScale) { uvScale_ = uvScale; }
	const Vector3& GetUVScale() const { return uvScale_; }

	void SetUVRotate(const Vector3& uvRotate) { uvRotate_ = uvRotate; }
	const Vector3& GetUVRotate() const { return uvRotate_; }

	void SetUVTranslate(const Vector3& uvTranslate) { uvTranslate_ = uvTranslate; }
	const Vector3& GetUVTranslate() const { return uvTranslate_; }
};
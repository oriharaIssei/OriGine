#pragma once

#include "SpriteCommon.h"
#include "Vector2.h"

struct SpriteVertexData {
    Vector4 pos;
    Vector2 texcoord;
};
struct SpritConstBuffer {
    Vector4 color_;
    Matrix4x4 mat_;
    Matrix4x4 uvMat_;
};
struct SpriteMesh {
    void Init();
    SpriteVertexData* vertexData = nullptr;
    uint32_t* indexData          = nullptr;

    DxResource vertBuff;
    DxResource indexBuff;

    D3D12_INDEX_BUFFER_VIEW ibView{};
    D3D12_VERTEX_BUFFER_VIEW vbView{};
};

class SpriteCommon;
class Sprite {
    friend class SpriteCommon;

public:
    Sprite(SpriteCommon* spriteCommon)
        : spriteCommon_(spriteCommon) {}
    ~Sprite() = default;

    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="texFilePath">TextureFilePath</param>
    void Init(const std::string& texFilePath);
    void Draw();

    void Debug(const std::string& name);

    void Update();
    void ConvertMappingData();

private:
    SpriteCommon* spriteCommon_;

    Vector2 textureLeftTop_ = {0.0f, 0.0f};
    Vector2 textureSize_    = {0.0f, 0.0f};

    Vector2 anchorPoint_;

    bool isFlipX_ = false;
    bool isFlipY_ = false;

    Vector2 size_ = {0.0f, 0.0f};
    float rotate_ = 0.0f;
    Vector2 pos_  = {0.0f, 0.0f};
    Matrix4x4 worldMat_;

    Vector3 uvScale_  = {1.0f, 1.0f, 1.0f};
    Vector3 uvRotate_ = {0.0f, 0.0f, 0.0f};
    Vector3 uvTranslate_;
    Matrix4x4 uvMat_;

    Vector4 color_ = {1.0f, 1.0f, 1.0f, 1.0f};

    SpritConstBuffer* mappingConstBufferData_;
    std::unique_ptr<SpriteMesh> meshBuff_;
    DxResource constBuff_;

    uint32_t textureIndex_ = 0;

public:
    void setSize(const Vector2& size) { size_ = size; }
    const Vector2& GetSize() const { return size_; }

    void setRotate(float rotate) { rotate_ = rotate; }
    float GetRotate() const { return rotate_; }

    void setPosition(const Vector2& pos) { pos_ = pos; }
    const Vector2& GetPosition() const { return pos_; }

    void setUVScale(const Vector3& uvScale) { uvScale_ = uvScale; }
    const Vector3& GetUVScale() const { return uvScale_; }

    void setUVRotate(const Vector3& uvRotate) { uvRotate_ = uvRotate; }
    const Vector3& GetUVRotate() const { return uvRotate_; }

    void setUVTranslate(const Vector3& uvTranslate) { uvTranslate_ = uvTranslate; }
    const Vector3& GetUVTranslate() const { return uvTranslate_; }

    void setAnchorPoint(const Vector2& anchor) { anchorPoint_ = anchor; }

    void setFlipX(bool flipX) { isFlipX_ = flipX; }
    void setFlipY(bool flipY) { isFlipY_ = flipY; }

    void setTextureLeftTop(const Vector2& lt) { textureLeftTop_ = lt; }
    const Vector2& getTextureLeftTop() const { return textureLeftTop_; }
    void setTextureSize(const Vector2& size) { textureSize_ = textureSize_; }
    const Vector2& getTextureSize() const { return textureSize_; }
};

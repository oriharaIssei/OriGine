#pragma once

#include "SpriteCommon.h"
#include "Vector2.h"

struct SpriteVertexData {
    Vec4f pos;
    Vec2f texcoord;
};
struct SpritConstBuffer {
    Vec4f color_;
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
    Sprite()
        : spriteCommon_(SpriteCommon::getInstance()) {}
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

    Vec2f textureLeftTop_ = {0.0f, 0.0f};
    Vec2f textureSize_    = {0.0f, 0.0f};

    Vec2f anchorPoint_;

    bool isFlipX_ = false;
    bool isFlipY_ = false;

    Vec2f size_ = {0.0f, 0.0f};
    float rotate_ = 0.0f;
    Vec2f pos_  = {0.0f, 0.0f};
    Matrix4x4 worldMat_;

    Vec3f uvScale_  = {1.0f, 1.0f, 1.0f};
    Vec3f uvRotate_ = {0.0f, 0.0f, 0.0f};
    Vec3f uvTranslate_;
    Matrix4x4 uvMat_;

    Vec4f color_ = {1.0f, 1.0f, 1.0f, 1.0f};

    SpritConstBuffer* mappingConstBufferData_;
    std::unique_ptr<SpriteMesh> meshBuff_;
    DxResource constBuff_;

    uint32_t textureIndex_ = 0;

public:
    void setSize(const Vec2f& size) { size_ = size; }
    const Vec2f& getSize() const { return size_; }

    void setRotate(float rotate) { rotate_ = rotate; }
    float getRotate() const { return rotate_; }

    void setPosition(const Vec2f& pos) { pos_ = pos; }
    const Vec2f& getPosition() const { return pos_; }

    void setUVScale(const Vec3f& uvScale) { uvScale_ = uvScale; }
    const Vec3f& getUVScale() const { return uvScale_; }

    void setUVRotate(const Vec3f& uvRotate) { uvRotate_ = uvRotate; }
    const Vec3f& getUVRotate() const { return uvRotate_; }

    void setUVTranslate(const Vec3f& uvTranslate) { uvTranslate_ = uvTranslate; }
    const Vec3f& getUVTranslate() const { return uvTranslate_; }

    void setColor(const Vec4f& color) { color_ = color; }
    const Vec4f& getColor() const { return color_; }

    void setAnchorPoint(const Vec2f& anchor) { anchorPoint_ = anchor; }

    void setFlipX(bool flipX) { isFlipX_ = flipX; }
    void setFlipY(bool flipY) { isFlipY_ = flipY; }

    void setTextureLeftTop(const Vec2f& lt) { textureLeftTop_ = lt; }
    const Vec2f& getTextureLeftTop() const { return textureLeftTop_; }
    void setTextureSize(const Vec2f& size) { textureSize_ = textureSize_; }
    const Vec2f& getTextureSize() const { return textureSize_; }
};

#pragma once

// directX12Object
#include "directX12/IConstantBuffer.h"
#include "directX12/Mesh.h"
// component
#include "component/renderer/MeshRenderer.h"

// math
#include <Vector2.h>
#include <Vector4.h>

//====================================== VertexData ======================================//
struct SpriteVertexData {
    SpriteVertexData()  = default;
    ~SpriteVertexData() = default;

    Vec4f pos;
    Vec2f texcoord;
};

//====================================== ConstBuffer ======================================//
struct SpritConstBuffer {
    SpritConstBuffer()  = default;
    ~SpritConstBuffer() = default;

    Vec4f color_ = {1.f, 1.f, 1.f, 1.f};

    Vec2f scale_        = {1.f, 1.f};
    float rotate_       = 0.f;
    Vec2f translate_    = {0.f, 0.f};
    Matrix4x4 worldMat_ = MakeMatrix::Identity();

    Vec2f uvScale_     = {1.f, 1.f};
    float uvRotate_    = 0.f;
    Vec2f uvTranslate_ = {0.f, 0.f};
    Matrix4x4 uvMat_   = MakeMatrix::Identity();

    void Update(const Matrix4x4& _vpMat) {
        worldMat_ = MakeMatrix::Affine({scale_, 1.0f}, {0.0f, 0.0f, rotate_}, {translate_, 0.0f}) * _vpMat;
        uvMat_    = MakeMatrix::Affine({uvScale_, 1.f}, {0.f, uvRotate_, 0.f}, {uvTranslate_, 0.f});
    }

    struct ConstantBuffer {
        ConstantBuffer()  = default;
        ~ConstantBuffer() = default;

        Vec4f color_;
        Matrix4x4 mat_;
        Matrix4x4 uvMat_;

        ConstantBuffer& operator=(const SpritConstBuffer& _sprit) {
            color_ = _sprit.color_;
            mat_   = _sprit.worldMat_;
            uvMat_ = _sprit.uvMat_;
            return *this;
        }
    };
};

//====================================== Mesh ======================================//
using SpriteMesh = Mesh<SpriteVertexData>;

//====================================== MeshRenderer ======================================//
class SpriteRenderer
    : public MeshRenderer<SpriteMesh, SpriteVertexData> {
    friend void to_json(nlohmann::json& j, const SpriteRenderer& r);
    friend void from_json(const nlohmann::json& j, SpriteRenderer& r);

public:
    SpriteRenderer() : MeshRenderer<SpriteMesh, SpriteVertexData>() {}
    ~SpriteRenderer() {}

    ///< summary>
    /// 初期化
    ///</summary>
    void Initialize(GameEntity* _hostEntity) override;

    void Edit(Scene* _scene, GameEntity* _entity, const std::string& _parentLabel) override;

    /// <summary>
    /// 更新
    /// </summary>
    void Update(const Matrix4x4& _viewPortMat);
    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize() override;

private:
    int32_t renderPriority_ = 1;

    IConstantBuffer<SpritConstBuffer> spriteBuff_;

    std::string texturePath_;
    uint32_t textureNumber_ = 0;

    Vec2f textureLeftTop_ = {0.0f, 0.0f};
    Vec2f textureSize_    = {0.0f, 0.0f};
    Vec2f size_           = {0.0f, 0.0f}; // px

    Vec2f anchorPoint_ = {0.0f, 0.0f};

    bool isFlipX_ = false;
    bool isFlipY_ = false;

public:
    uint32_t getTextureNumber() const { return textureNumber_; }
    void setTexture(const std::string& _texturePath, bool _applyTextureSize);

    int32_t getRenderPriority() const { return renderPriority_; }
    void setRenderPriority(int32_t num) { renderPriority_ = num; }

    const IConstantBuffer<SpritConstBuffer>& getSpriteBuff() const { return spriteBuff_; }
    IConstantBuffer<SpritConstBuffer>& getSpriteBuff() { return spriteBuff_; }

    void setSize(const Vec2f& size) { size_ = size; }
    const Vec2f& getSize() const { return size_; }

    void setScale(const Vec2f& scale) { spriteBuff_->scale_ = scale; }
    const Vec2f& getScale() const { return spriteBuff_->scale_; }

    void setRotate(float rotate) { spriteBuff_->rotate_ = rotate; }
    float getRotate() const { return spriteBuff_->rotate_; }

    void setTranslate(const Vec2f& _translate) { spriteBuff_->translate_ = _translate; }
    const Vec2f& getTranslate() const { return spriteBuff_->translate_; }

    void setUVScale(const Vec2f& uvScale) { spriteBuff_->uvScale_ = uvScale; }
    const Vec2f& getUVScale() const { return spriteBuff_->uvScale_; }

    void setUVRotate(float uvRotate) { spriteBuff_->uvRotate_ = uvRotate; }
    float getUVRotate() const { return spriteBuff_->uvRotate_; }

    void setUVTranslate(const Vec2f& uvTranslate) { spriteBuff_->uvTranslate_ = uvTranslate; }
    const Vec2f& getUVTranslate() const { return spriteBuff_->uvTranslate_; }

    void setColor(const Vec4f& color) { spriteBuff_->color_ = color; }
    const Vec4f& getColor() const { return spriteBuff_->color_; }

    const Vec2f& getAnchorPoint() const { return anchorPoint_; }
    void setAnchorPoint(const Vec2f& anchor) { anchorPoint_ = anchor; }

    void setFlipX(bool flipX) { isFlipX_ = flipX; }
    void setFlipY(bool flipY) { isFlipY_ = flipY; }

    void setTextureLeftTop(const Vec2f& lt) { textureLeftTop_ = lt; }
    const Vec2f& getTextureLeftTop() const { return textureLeftTop_; }
    void setTextureSize(const Vec2f& size) { textureSize_ = size; }
    const Vec2f& getTextureSize() const { return textureSize_; }
};

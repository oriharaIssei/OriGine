#pragma once

// directX12Object
#include "directX12/IConstantBuffer.h"
#include "directX12/Mesh.h"
// component
#include "component/renderer/MeshRender.h"

// math
#include "Vector2.h"

//====================================== VertexData ======================================//
struct SpriteVertexData {
    Vec4f pos;
    Vec2f texcoord;
};

//====================================== ConstBuffer ======================================//
struct SpritConstBuffer {
    Vec4f color_ = {1.f, 1.f, 1.f, 1.f};

    Vec2f scale_     = {0.f, 0.f};
    float rotate_    = 0.f;
    Vec2f translate_ = {0.f, 0.f};
    Matrix4x4 worldMat_;

    Vec2f uvScale_     = {1.f, 1.f};
    float uvRotate_    = 0.f;
    Vec2f uvTranslate_ = {0.f, 0.f};
    Matrix4x4 uvMat_;

    void Update(const Matrix4x4& _vpMat) {
        worldMat_ = MakeMatrix::Affine({scale_, 1.0f}, {0.0f, 0.0f, rotate_}, {translate_, 0.0f}) * _vpMat;
        uvMat_    = MakeMatrix::Affine({uvScale_, 1.f}, {0.f, uvRotate_, 0.f}, {uvTranslate_, 0.f});
    }

    struct ConstantBuffer {
        Matrix4x4 mat_;
        Vec4f color_;

        ConstantBuffer& operator=(const SpritConstBuffer& _sprit) {
            mat_   = _sprit.worldMat_;
            color_ = _sprit.color_;
            return *this;
        }
    };
};

//====================================== Mesh ======================================//
using SpriteMesh = Mesh<SpriteVertexData>;

//====================================== MeshRenderer ======================================//
class SpriteRenderer
    : public MeshRenderer<SpriteMesh, SpriteVertexData> {
public:
    SpriteRenderer() : MeshRenderer<SpriteMesh, SpriteVertexData>() {}
    ~SpriteRenderer() {}

    ///< summary>
    /// 初期化
    ///</summary>
    void Initialize(GameEntity* _hostEntity) override;

    bool Edit() override;
    void Save(BinaryWriter& _writer) override {
        MeshRenderer<SpriteMesh, SpriteVertexData>::Save(_writer);

        _writer.Write<uint32_t>(renderingNum_);

        _writer.Write(texturePath_);

        _writer.Write<2,float>(textureLeftTop_);
        _writer.Write<2,float>(textureSize_);
        _writer.Write<2,float>(anchorPoint_);

        _writer.Write<bool>(isFlipX_);
        _writer.Write<bool>(isFlipY_);
    }
    void Load(BinaryReader& _reader) override {
        MeshRenderer<SpriteMesh, SpriteVertexData>::Load(_reader);

        _reader.Read<uint32_t>(renderingNum_);

        _reader.Read(texturePath_);
        if (!texturePath_.empty()) {
            textureNumber_ = TextureManager::LoadTexture(texturePath_);
        }

        _reader.Read<2,float>(textureLeftTop_);
        _reader.Read<2,float>(textureSize_);
        _reader.Read<2,float>(anchorPoint_);

        _reader.Read<bool>(isFlipX_);
        _reader.Read<bool>(isFlipY_);
    }

    /// <summary>
    /// 更新
    /// </summary>
    void Update(const Matrix4x4& _viewPortMat);
    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize() override;

private:
    uint32_t renderingNum_ = 1;

    IConstantBuffer<SpritConstBuffer> spriteBuff_;

    std::string texturePath_;
    uint32_t textureNumber_ = 0;

    Vec2f textureLeftTop_ = {0.0f, 0.0f};
    Vec2f textureSize_    = {0.0f, 0.0f};

    Vec2f anchorPoint_ = {0.0f, 0.0f};

    bool isFlipX_ = false;
    bool isFlipY_ = false;

public:
    uint32_t getTextureNumber() const { return textureNumber_; }

    uint32_t getRenderingNum() const { return renderingNum_; }
    void setRenderingNum(uint32_t num) { renderingNum_ = num; }

    const IConstantBuffer<SpritConstBuffer>& getSpriteBuff() const { return spriteBuff_; }
    IConstantBuffer<SpritConstBuffer>& getSpriteBuff() { return spriteBuff_; }

    void setSize(const Vec2f& size) { spriteBuff_->scale_ = size; }
    const Vec2f& getSize() const { return spriteBuff_->scale_; }

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

    void setAnchorPoint(const Vec2f& anchor) { anchorPoint_ = anchor; }

    void setFlipX(bool flipX) { isFlipX_ = flipX; }
    void setFlipY(bool flipY) { isFlipY_ = flipY; }

    void setTextureLeftTop(const Vec2f& lt) { textureLeftTop_ = lt; }
    const Vec2f& getTextureLeftTop() const { return textureLeftTop_; }
    void setTextureSize(const Vec2f& size) { textureSize_ = size; }
    const Vec2f& getTextureSize() const { return textureSize_; }
};

//====================================== SpriteMeshController ======================================//
// class SpriteRendererController
//    : public RendererComponentController<SpriteRenderer> {
// public:
//    SpriteRendererController() : RendererComponentController<SpriteRenderer>() {}
//    ~SpriteRendererController() {}
//
//    ///< summary>
//    /// 初期化
//    ///</summary>
//    void Initialize() override;
//
// protected:
//    ///< summary>
//    /// 更新
//    ///</summary>
//    void Update() override;
//    ///< summary>
//    /// 描画開始
//    ///</summary>
//    void StartRender() override;
//    ///< summary>
//    /// 描画終了
//    ///</summary>
//    void EndRender() override{}
//
//    ///< summary>
//    /// PipelineStateObjectを作成
//    ///</summary>
//    void CreatePso() override;
//
// private:
//    Matrix4x4 viewPortMat_;
//
// public:
//    const Matrix4x4& getViewPortMat() const { return viewPortMat_; }
//};

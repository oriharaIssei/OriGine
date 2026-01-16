#pragma once

// directX12Object
#include "directX12/buffer/IConstantBuffer.h"
#include "directX12/mesh/Mesh.h"
// component
#include "component/renderer/MeshRenderer.h"
#include "component/transform/Transform2d.h"

// math
#include <Vector2.h>
#include <Vector4.h>

namespace OriGine {

//====================================== VertexData ======================================//
///< summary>
/// スプライト頂点データ
///</summary>
struct SpriteVertexData {
    SpriteVertexData()  = default;
    ~SpriteVertexData() = default;

    Vec4f pos;
    Vec2f texcoord;
};

//====================================== ConstBuffer ======================================//
///< summary>
/// スプライト定数バッファ
///</summary>
struct SpritConstBuffer {
    SpritConstBuffer()  = default;
    ~SpritConstBuffer() = default;

    Vec4f color_ = {1.f, 1.f, 1.f, 1.f};

    Vec2f scale_        = {1.f, 1.f};
    float rotate_       = 0.f;
    Vec2f translate_    = {0.f, 0.f};
    Matrix4x4 worldMat_ = MakeMatrix4x4::Identity();

    Vec2f uvScale_     = {1.f, 1.f};
    float uvRotate_    = 0.f;
    Vec2f uvTranslate_ = {0.f, 0.f};
    Matrix4x4 uvMat_   = MakeMatrix4x4::Identity();

    void Update(const Matrix4x4& _vpMat) {
        worldMat_ = MakeMatrix4x4::Affine({scale_, 1.0f}, {0.0f, 0.0f, rotate_}, {translate_, 0.0f}) * _vpMat;
        uvMat_    = MakeMatrix4x4::Affine({uvScale_, 1.f}, {0.f, uvRotate_, 0.f}, {uvTranslate_, 0.f});
    }

    /// <summary>
    /// スプライト定数バッファ -> 定数バッファのみの構造体変換
    /// </summary>
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
///< summary>
/// スプライト描画コンポーネント
///</summary>
class SpriteRenderer
    : public MeshRenderer<SpriteMesh, SpriteVertexData> {
    friend void to_json(nlohmann::json& _j, const SpriteRenderer& _comp);
    friend void from_json(const nlohmann::json& _j, SpriteRenderer& _comp);

public:
    SpriteRenderer() : MeshRenderer<SpriteMesh, SpriteVertexData>() {}
    ~SpriteRenderer() {}

    ///< summary>
    /// 初期化
    ///</summary>
    void Initialize(Scene* _scene, EntityHandle _hostEntity) override;

    void Edit(Scene* _scene, EntityHandle _entity, const std::string& _parentLabel) override;

    /// <summary>
    /// バッファ更新
    /// </summary>
    void UpdateBuffer(const Matrix4x4& _viewPortMat);
    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize() override;

    /// <summary>
    /// ウィンドウ比率に基づいて位置とサイズを計算
    /// </summary>
    void CalculateWindowRatioPosAndSize();
    /// <summary>
    /// ウィンドウサイズ変更に基づいて位置とサイズを計算
    /// </summary>
    /// <param name="_newWindowSize">新しいウィンドウサイズ</param>
    void CalculateWindowRatioPosAndSize(const Vec2f& _newWindowSize);
    /// <summary>
    /// 位置とサイズからウィンドウ比率を計算
    /// </summary>
    void CalculatePosRatioAndSizeRatio();
    /// <summary>
    /// 位置とサイズからウィンドウ比率を計算
    /// </summary>
    void CalculatePosRatioAndSizeRatio(const Vec2f& _newWindowSize);

private:
    int32_t renderPriority_ = 1;

    IConstantBuffer<Transform2d> transform2dBuff_;
    IConstantBuffer<Transform2d> uvTransform2dBuff_;
    IConstantBuffer<SpritConstBuffer> spriteBuff_;

    std::string texturePath_;
    uint32_t textureNumber_ = 0;

    Vec2f textureLeftTop_ = {0.0f, 0.0f};
    Vec2f textureSize_    = {0.0f, 0.0f};
    Vec2f size_           = {0.0f, 0.0f}; // px

    Vec2f defaultWindowSize_ = {0.f, 0.f}; // 画面の基準サイズ(これに対する比率で位置とサイズを決定)
    Vec2f windowRatioPos_    = {0.f, 0.f}; // 画面のサイズに対する位置(基本 0 ~ 1)
    Vec2f windowRatioSize_   = {0.f, 0.f}; // 画面のサイズに対するサイズ(基本 0 ~ 1)

    Vec2f anchorPoint_ = {0.0f, 0.0f};

    bool isFlipX_ = false;
    bool isFlipY_ = false;

public:
    uint32_t GetTextureNumber() const { return textureNumber_; }
    void SetTexture(const std::string& _texturePath, bool _applyTextureSize);

    int32_t GetRenderPriority() const { return renderPriority_; }
    void SetRenderPriority(int32_t _num) { renderPriority_ = _num; }

    const IConstantBuffer<SpritConstBuffer>& GetSpriteBuff() const { return spriteBuff_; }
    IConstantBuffer<SpritConstBuffer>& GetSpriteBuff() { return spriteBuff_; }

    void SetSize(const Vec2f& _size) { size_ = _size; }
    const Vec2f& GetSize() const { return size_; }

    void SetScale(const Vec2f& _scale) { spriteBuff_->scale_ = _scale; }
    const Vec2f& GetScale() const { return spriteBuff_->scale_; }

    void SetRotate(float _rotate) { spriteBuff_->rotate_ = _rotate; }
    float GetRotate() const { return spriteBuff_->rotate_; }

    void SetTranslate(const Vec2f& _translate) { spriteBuff_->translate_ = _translate; }
    const Vec2f& GetTranslate() const { return spriteBuff_->translate_; }

    void SetUVScale(const Vec2f& _uvScale) { spriteBuff_->uvScale_ = _uvScale; }
    const Vec2f& GetUVScale() const { return spriteBuff_->uvScale_; }

    void SetUVRotate(float _uvRotate) { spriteBuff_->uvRotate_ = _uvRotate; }
    float GetUVRotate() const { return spriteBuff_->uvRotate_; }

    void SetUVTranslate(const Vec2f& _uvTranslate) { spriteBuff_->uvTranslate_ = _uvTranslate; }
    const Vec2f& GetUVTranslate() const { return spriteBuff_->uvTranslate_; }

    const Vec4f& GetColor() const { return spriteBuff_->color_; }
    void SetColor(const Vec4f& _color) { spriteBuff_->color_ = _color; }

    const Vec2f& GetAnchorPoint() const { return anchorPoint_; }
    void SetAnchorPoint(const Vec2f& _anchor) { anchorPoint_ = _anchor; }

    void SetFlipX(bool _flipX) { isFlipX_ = _flipX; }
    void SetFlipY(bool _flipY) { isFlipY_ = _flipY; }

    void SetTextureLeftTop(const Vec2f& _lt) { textureLeftTop_ = _lt; }
    const Vec2f& GetTextureLeftTop() const { return textureLeftTop_; }
    void SetTextureSize(const Vec2f& _size) { textureSize_ = _size; }
    const Vec2f& GetTextureSize() const { return textureSize_; }
};

} // namespace OriGine

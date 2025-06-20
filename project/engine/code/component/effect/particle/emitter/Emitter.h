#pragma once

/// stl
#include <memory>
#include <vector>

/// engine
// dx12object
#include "directX12/IConstantBuffer.h"
#include "directX12/IStructuredBuffer.h"
#include "directX12/Mesh.h"
#include "directX12/ShaderManager.h"
// assets
#include "component/material/Material.h"
struct ParticleKeyFrames;
struct EmitterShape;
// object
#include "../Particle.h"
// component
#include "component/transform/CameraTransform.h"
#include "component/transform/ParticleTransform.h"
#include "component/transform/Transform.h"

/// math
// shape
#include "EmitterShape.h"

enum class BillBoardType {
    NONE = 0,
    X    = 0x1 << 1,
    Y    = 0x1 << 2,
    Z    = 0x1 << 3
};

class Emitter
    : public IComponent {

    // to_json, from_json を friend として宣言
    friend void to_json(nlohmann::json& j, const Emitter& e);
    friend void from_json(const nlohmann::json& j, Emitter& e);

public:
    Emitter();
    ~Emitter();

    void Initialize(GameEntity* _entity) override;
    //! ToDo :: Update,Draw の削除
    void Update(float _deltaTime);
    void UpdateParticle(float _deltaTime);
    void Draw(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList);

    void Finalize() override;

    bool Edit() override;

    void CalculateMaxSize();

    void PlayStart();
    void PlayContinue();
    void PlayStop();

private:
    void CreateResource();

    void SpawnParticle();

#ifdef _DEBUG
    void EditEmitter();

    /// <summary>
    /// ShapeType に関する 編集項目
    /// </summary>
    void EditShapeType();

    /// <summary>
    /// Particle の 編集項目
    /// </summary>
    void EditParticle();
#endif // _DEBUG
private:
    Transform* parent_       = nullptr;
    Vec3f preWorldOriginPos_ = {0.f, 0.f, 0.f};
    Vec3f worldOriginPos_    = {0.f, 0.f, 0.f};
    Vec3f originPos_         = {0.f, 0.f, 0.f};

    uint32_t particleMaxSize_ = 12;

    std::vector<std::shared_ptr<Particle>> particles_;

    /// <summary>
    /// 頂点とMaterial を 併せ持つ
    /// </summary>
    TextureMesh mesh_;
    IConstantBuffer<Material> material_;

    IStructuredBuffer<ParticleTransform> structuredTransform_;
    //=============== Texture ===============/
    std::string textureFileName_ = "";
    int32_t textureIndex_        = 0;

    //=============== エミッター設定項目 ===============//
    BlendMode blendMode_ = BlendMode::None;
    bool isActive_       = false;
    bool isLoop_         = false;
    // emitter 生存時間
    float activeTime_     = 0.f;
    float leftActiveTime_ = 0.f;

    /// <summary>
    /// 一度に 生成される Particle の 数
    /// </summary>
    int32_t spawnParticleVal_ = 1;

    EmitterShapeType shapeType_ = EmitterShapeType::SPHERE;
    std::shared_ptr<EmitterShape> emitterSpawnShape_;

    float currentCoolTime_  = 0.f;
    float spawnCoolTime_    = 0.f;
    float particleLifeTime_ = 0.f;

    // billBoard 計算するかどうか
    bool particleIsBillBoard_ = true;

    //=============== パーティクル設定項目 ===============//
    Vec4f particleColor_   = {1.f, 1.f, 1.f, 1.f};
    Vec3f particleUvScale_ = {1.f, 1.f, 1.f};
    Vec3f particleUvRotate_;
    Vec3f particleUvTranslate_;

    int32_t updateSettings_ = 0;

    Vec2f randMass_ = {1.f, 1.f};

    std::shared_ptr<ParticleKeyFrames> particleKeyFrames_ = nullptr;

#ifdef _DEBUG
    // 連番画像から uv Curveにするためのもの
    Vec2f tileSize_            = {};
    Vec2f textureSize_         = {};
    float tilePerTime_         = 0.f;
    float startAnimationTime_  = 0.f;
    float animationTimeLength_ = 0.f;
#endif // _DEBUG

    InterpolationType transformInterpolationType_ = InterpolationType::LINEAR;
    InterpolationType colorInterpolationType_     = InterpolationType::LINEAR;
    InterpolationType uvInterpolationType_        = InterpolationType::LINEAR;
    // ランダムな数値の範囲を設定するためのメンバ変数
    // ランダムではない場合 (min == max) になる
    Vec3f startParticleScaleMin_    = {1.f, 1.f, 1.f};
    Vec3f startParticleScaleMax_    = {1.f, 1.f, 1.f};
    Vec3f startParticleRotateMin_   = {0.f, 0.f, 0.f};
    Vec3f startParticleRotateMax_   = {0.f, 0.f, 0.f};
    Vec3f startParticleVelocityMin_ = {0.f, 0.f, 0.f};
    Vec3f startParticleVelocityMax_ = {0.f, 0.f, 0.f};

    Vec3f updateParticleScaleMin_    = {1.f, 1.f, 1.f};
    Vec3f updateParticleScaleMax_    = {1.f, 1.f, 1.f};
    Vec3f updateParticleRotateMin_   = {0.f, 0.f, 0.f};
    Vec3f updateParticleRotateMax_   = {0.f, 0.f, 0.f};
    Vec3f updateParticleVelocityMin_ = {0.f, 0.f, 0.f};
    Vec3f updateParticleVelocityMax_ = {0.f, 0.f, 0.f};

public:
    bool getIsActive() const { return isActive_; }
    // void setIsActive(bool _isActive) { isActive_ = _isActive; } // PlayStart を使え
    bool getIsLoop() const { return isLoop_; }
    void setIsLoop(bool _isLoop) { isLoop_ = _isLoop; }

    bool getIsBillBoard() const { return particleIsBillBoard_; }
    void setIsBillBoard(bool _isBillBoard) { particleIsBillBoard_ = _isBillBoard; }

    const Vec3f& getOriginePos() const { return originPos_; }
    void setOriginePos(const Vec3f& _pos) { originPos_ = _pos; }

    BlendMode getBlendMode() const { return blendMode_; }

    void setParent(Transform* _parent) { parent_ = _parent; }
};

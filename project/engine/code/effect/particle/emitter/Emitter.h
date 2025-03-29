#pragma once

/// stl
#include <memory>
#include <vector>

/// engine
// dx12object
#include "directX12/IConstantBuffer.h"
#include "directX12/IStructuredBuffer.h"
#include "directX12/ShaderManager.h"
// assets
#include "component/material/Material.h"
struct ParticleKeyFrames;
struct EmitterShape;
struct ParticleKeyFrames;
// object
#include "../Particle.h"
// transform
#include "component/transform/CameraTransform.h"
#include "component/transform/ParticleTransform.h"
#include "component/transform/Transform.h"

/// math
// shape
#include "EmitterShape.h"

class Emitter
    : public IComponent {
    friend class ParticleEditor;

public:
    Emitter(DxSrvArray* _srvArray);
    ~Emitter();

    void Initialize(GameEntity* _entity) override;
    //! ToDo :: Update,Draw の削除
    void Update(float _deltaTime);
    void Draw(ID3D12GraphicsCommandList* _commandList);

    void Finalize() override;

    bool Edit() override;
    void Save(BinaryWriter& _writer) override;
    void Load(BinaryReader& _reader) override;

    void CalculateMaxSize();

private:
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
    DxSrvArray* srvArray_ = nullptr;

    // .pkf ファイルのパス
    std::string particleKeyFrameFileName_;

    Transform* parent_ = nullptr;
    Vec3f originPos_;
    uint32_t particleMaxSize_;

    std::vector<std::shared_ptr<Particle>> particles_;

    /// <summary>
    /// 頂点とMaterial を 併せ持つ
    /// </summary>
    std::shared_ptr<Model> particleModel_;
    IStructuredBuffer<ParticleTransform> structuredTransform_;
    //=============== Model & Texture ===============/
    std::string modelFileName_;
    std::string textureFileName_;
    int32_t textureIndex_;

    //=============== エミッター設定項目 ===============//
    BlendMode blendMode_;
    bool isActive_;
    bool isLoop_;
    // emitter 生存時間
    float activeTime_;
    float leftActiveTime_;

    /// <summary>
    /// 一度に 生成される Particle の 数
    /// </summary>
    int32_t spawnParticleVal_;

    EmitterShapeType shapeType_;
    std::shared_ptr<EmitterShape> emitterSpawnShape_;

    float currentCoolTime_  = 0.f;
    float spawnCoolTime_    = 0.f;
    float particleLifeTime_ = 0.f;

    // billBoard 計算するかどうか
    bool particleIsBillBoard_;

    //=============== パーティクル設定項目 ===============//
    Vec4f particleColor_;
    Vec3f particleUvScale_;
    Vec3f particleUvRotate_;
    Vec3f particleUvTranslate_;

    int32_t updateSettings_;

    std::unique_ptr<ParticleKeyFrames> particleKeyFrames_ = nullptr;

    // ランダムな数値の範囲を設定するためのメンバ変数
    // ランダムではない場合 (min == max) になる
    Vec3f startParticleScaleMin_;
    Vec3f startParticleScaleMax_;
    Vec3f startParticleRotateMin_;
    Vec3f startParticleRotateMax_;
    Vec3f startParticleVelocityMin_;
    Vec3f startParticleVelocityMax_;

    Vec3f updateParticleScaleMin_;
    Vec3f updateParticleScaleMax_;
    Vec3f updateParticleRotateMin_;
    Vec3f updateParticleRotateMax_;
    Vec3f updateParticleVelocityMin_;
    Vec3f updateParticleVelocityMax_;

public:
    bool getIsActive() const { return isActive_; };
};

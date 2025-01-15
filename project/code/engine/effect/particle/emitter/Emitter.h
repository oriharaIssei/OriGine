#pragma once

///stl
#include <memory>
#include <vector>

///engine
//dx12object
#include "directX12/IConstantBuffer.h"
#include "directX12/IStructuredBuffer.h"
//assets
#include "material/Material.h"
struct ParticleKeyFrames;
struct EmitterShape;
struct ParticleKeyFrames;
//object
#include "../Particle.h"
#include "object3d/Object3d.h"
//transform
#include "transform/CameraTransform.h"
#include "transform/ParticleTransform.h"
#include "transform/Transform.h"
//lib
#include "globalVariables/SerializedField.h"

class Emitter {
public:
    Emitter(DxSrvArray* srvArray, const std::string& emitterName);
    ~Emitter();

    void Init();
    void Update(float deltaTime);

    void Draw();

#ifdef _DEBUG
    void Debug();
#endif // _DEBUG

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
    std::string emitterName_;
    DxSrvArray* srvArray_ = nullptr;

    Vector3 originPos_;
    uint32_t particleMaxSize_;

    std::vector<std::unique_ptr<Particle>> particles_;

    /// <summary>
    /// 頂点とMaterial を 併せ持つ
    /// </summary>
    std::unique_ptr<Model> particleModel_;
    IStructuredBuffer<ParticleTransform> structuredTransform_;
    //=============== 形状設定項目 ===============//
    SerializedField<std::string> modelDirectory_;
    SerializedField<std::string> textureDirectory_;
    SerializedField<std::string> modelFileName_;
    SerializedField<std::string> textureFileName_;

    //=============== エミッター設定項目 ===============//
    SerializedField<int32_t> blendMode_;
    bool isActive_;
    SerializedField<bool> isLoop_;
    // emitter 生存時間
    SerializedField<float> activeTime_;
    float leftActiveTime_;

    /// <summary>
    /// 一度に 生成される Particle の 数
    /// </summary>
    SerializedField<int32_t> spawnParticleVal_;

    SerializedField<int32_t> shapeType_;
    std::unique_ptr<EmitterShape> emitterSpawnShape_;

    float currentCoolTime_ = 0.0f;
    SerializedField<float> spawnCoolTime_;
    SerializedField<float> particleLifeTime_;

    // billBoard 計算するかどうか
    SerializedField<bool> particleIsBillBoard_;

    //=============== パーティクル設定項目 ===============//
    SerializedField<Vector4> particleColor_;
    SerializedField<Vector3> particleScale_;
    SerializedField<Vector3> particleRotate_;
    SerializedField<float> particleSpeed_;

    SerializedField<Vector3> particleUvScale_;
    SerializedField<Vector3> particleUvRotate_;
    SerializedField<Vector3> particleUvTranslate_;

    SerializedField<int32_t> updateSettings_;

    std::unique_ptr<ParticleKeyFrames> particleKeyFrames_ = nullptr;

public:
    bool getIsActive() const { return isActive_; }
};

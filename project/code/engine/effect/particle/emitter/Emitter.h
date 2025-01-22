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
    friend class ParticleEditor;

public:
    Emitter(DxSrvArray* srvArray, const std::string& emitterName, int _id);
    ~Emitter();

    void Init();
    void Update(float deltaTime);
    void Draw();
    void Finalize();

#ifdef _DEBUG
    void Debug();

    void Save();
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
    // jsonのファイル名と一致する名前
    std::string emitterDataName_;
    // 個別のEmitterを識別するための名前
    int id_;

    DxSrvArray* srvArray_ = nullptr;

    Vec3f originPos_;
    uint32_t particleMaxSize_;

    std::vector<std::unique_ptr<Particle>> particles_;

    /// <summary>
    /// 頂点とMaterial を 併せ持つ
    /// </summary>
    std::unique_ptr<Model> particleModel_;
    IStructuredBuffer<ParticleTransform> structuredTransform_;
    //=============== 形状設定項目 ===============/
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
    SerializedField<Vec4f> particleColor_;
    SerializedField<Vec3f> particleUvScale_;
    SerializedField<Vec3f> particleUvRotate_;
    SerializedField<Vec3f> particleUvTranslate_;

    SerializedField<int32_t> updateSettings_;

    std::unique_ptr<ParticleKeyFrames> particleKeyFrames_ = nullptr;

    // ランダムな数値の範囲を設定するためのメンバ変数
    // ランダムではない場合 (min == max) になる
    SerializedField<Vec3f> startParticleScaleMin_;
    SerializedField<Vec3f> startParticleScaleMax_;
    SerializedField<Vec3f> startParticleRotateMin_;
    SerializedField<Vec3f> startParticleRotateMax_;
    SerializedField<Vec3f> startParticleVelocityMin_;
    SerializedField<Vec3f> startParticleVelocityMax_;

    SerializedField<Vec3f> updateParticleScaleMin_;
    SerializedField<Vec3f> updateParticleScaleMax_;
    SerializedField<Vec3f> updateParticleRotateMin_;
    SerializedField<Vec3f> updateParticleRotateMax_;
    SerializedField<Vec3f> updateParticleVelocityMin_;
    SerializedField<Vec3f> updateParticleVelocityMax_;

public:
    bool getIsActive() const { return isActive_; };
    const std::string& getDataName() const { return emitterDataName_; }
    int getId() const { return id_; }
};

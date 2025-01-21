#pragma once

///stl
//memory
#include <memory>
//container
#include <vector>
//string
#include <string>

///engine
#include "animation/Animation.h"
//dxObject
class DxSrvArray;
//Object
class Emitter;
class ISilhouette;
//lib
#include "globalVariables/SerializedField.h"

/// <summary>
/// 1 エフェクトを表すクラス
/// 複数種類の Particle で 1つのエフェクトを表すためのクラス
/// </summary>
class Effect {
public:
    Effect(std::shared_ptr<DxSrvArray> _srvArray, const std::string& _name);
    ~Effect();

    void Init();
    void Update(float _deltaTime);
    void Draw();
    void Finalize();

    void Debug();

private:
    void LoadEffect();
    void SaveEffect();

    void StartEmitter();

private:
    std::string name_;

    Vec3f origen_;

    int currentEmitterIndex_ = 0;

    // 使用するSRVの数 = Emitterの数
    //int32_t usingSrvNum_                  = 0;
    std::shared_ptr<DxSrvArray> srvArray_ = nullptr;

    bool isActive_                = false;
    SerializedField<bool> isLoop_ = SerializedField<bool>::CreateNull();

    SerializedField<float> duration_ = SerializedField<float>::CreateNull();
    float currentTime_               = 0.0f;

    std::vector<Keyframe<int>> particleSchedule_;
    std::vector<std::unique_ptr<Emitter>> emitters_;
    std::vector<Emitter*> activeEmitters_;

public:
    const Vec3f getOrigen() const { return origen_; }
    void setOrigen(const Vec3f& _origen) { origen_ = _origen; }

    int32_t getUsingSrvNum() const { return emitters_.size(); }

};

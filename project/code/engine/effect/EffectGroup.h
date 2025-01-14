#pragma once

///stl
//memory
#include <memory>
//container
#include <vector>
//string
#include <string>

///engine
//Object
class Emitter;
class AnimationObject3d;

/// <summary>
/// 1 エフェクトを表すクラス
/// 複数種類の Particle や Model(Animation) で 1つのエフェクトを表すためのクラス
/// </summary>
class EffectGroup {
public:
    EffectGroup(const std::string& _name);
    ~EffectGroup();
    void Init();
    void Update();

private:
    void LoadEffectGroup();
    void SaveEffectGroup();

private:
    std::string name_;
    std::vector<std::unique_ptr<Emitter>> emitters_; 
};

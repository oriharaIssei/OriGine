#pragma once

#include "Quaternion.h"
#include "Vector3.h"
#include "globalVariables/SerializedField.h"

#include <list>
#include <memory>
#include <string>
#include <vector>

class AnimationObject3d;

/// <summary>
/// アニメーションのエディター
/// </summary>
class AnimationEditor {
public:
    AnimationEditor();
    ~AnimationEditor();

    void Init();
    void Update();
    void DrawEditObject();

private:
    struct AnimationSetting {
        AnimationSetting(const std::string& _name)
            : name(_name.c_str()),
              targetModelDirection_("Animations", _name, "targetModelDirection"),
              targetModelFileName_("Animations", _name, "targetModelFileName") {}
        ~AnimationSetting() {}
        // アニメーション名
        std::string name;
        // アニメーション対象のモデル名
        SerializedField<std::string> targetModelDirection_;
        SerializedField<std::string> targetModelFileName_;
    };

private:
    // アニメーションリスト
    std::list<std::pair<std::string, std::string>> animationSettingsFileList_;
    std::list<std::pair<std::string, std::string>> modelFileList_;

    // アニメーション対象のオブジェクト
    std::unique_ptr<AnimationObject3d> currentEditObject_;
    std::unique_ptr<AnimationSetting> currentEditAnimationSetting_;

    bool isObjectPlaying_ = false;
};

#pragma once

#include "Quaternion.h"
#include "Vector3.h"
#include "globalVariables/SerializedField.h"
#include "module/editor/IEditor.h"

#include <list>
#include <memory>
#include <string>
#include <vector>

class AnimationObject3d;
struct AnimationData;
struct AnimationSetting;

/// <summary>
/// アニメーションのエディター
/// </summary>
class AnimationEditor
    : public IEditor {
public:
    AnimationEditor();
    ~AnimationEditor();

    void Init();
    void Update() override;
    void DrawEditObject();

private:
    // アニメーションリスト
    std::list<std::pair<std::string, std::string>> animationSettingsFileList_;
    std::list<std::pair<std::string, std::string>> modelFileList_;

    // アニメーション対象のオブジェクト
    std::unique_ptr<AnimationObject3d> currentEditObject_          = nullptr;
    std::unique_ptr<AnimationSetting> currentEditAnimationSetting_ = nullptr;

    bool isObjectPlaying_ = false;
};

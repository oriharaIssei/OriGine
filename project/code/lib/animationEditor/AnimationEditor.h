#pragma once

#include "Quaternion.h"
#include "Vector3.h"

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
#ifdef _DEBUG
    void Update();
#endif // _DEBUG
    void DrawEditObject();

private:
    // アニメーションリスト
    std::list<std::pair<std::string, std::string>> animationFileList_;
    // 現在編集中のアニメーションファイル名
    std::pair<std::string, std::string> currentEditAnimaitonFileName_;
    // アニメーション対象のオブジェクト
    std::unique_ptr<AnimationObject3d> currentEditObject_;

    bool isObjectPlaying_ = false;
};

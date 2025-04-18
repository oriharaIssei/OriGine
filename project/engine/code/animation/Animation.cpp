#include "Animation.h"

//assets
#include "model/Model.h"

//math
#include <cmath>

void Animation::Update(float deltaTime, Model* model, const Matrix4x4& parentTransform) {
    {
        isEnd_ = false;
        // 時間更新
        currentAnimationTime += deltaTime;

        // リピート
        if (currentAnimationTime > duration) {
            isEnd_               = true;
            currentAnimationTime = std::fmod(currentAnimationTime, duration);
        }
    }

    {
        ApplyAnimationToNodes(model->meshData_->rootNode, parentTransform, this);
    }
}

Matrix4x4 Animation::CalculateNodeLocal(const std::string& nodeName) const {
    auto it = data->nodeAnimations.find(nodeName);
    if (it == data->nodeAnimations.end()) {
        // ノードに対応するアニメーションがない場合、単位行列を返す
        return MakeMatrix::Identity();
    }

    const NodeAnimation& nodeAnimation = it->second;

    Vec3f scale;
    Quaternion rotate;
    Vec3f translate;

    switch (nodeAnimation.interpolationType) {
    case InterpolationType::LINEAR:
        scale     = CalculateValue::LINEAR(nodeAnimation.scale, currentAnimationTime);
        rotate    = Quaternion::Normalize(CalculateValue::LINEAR(nodeAnimation.rotate, currentAnimationTime));
        translate = CalculateValue::LINEAR(nodeAnimation.translate, currentAnimationTime);
        break;
    case InterpolationType::STEP:
        scale     = CalculateValue::Step(nodeAnimation.scale, currentAnimationTime);
        rotate    = Quaternion::Normalize(CalculateValue::Step(nodeAnimation.rotate, currentAnimationTime));
        translate = CalculateValue::Step(nodeAnimation.translate, currentAnimationTime);
        break;
    }

    return MakeMatrix::Affine(scale, rotate, translate);
}

float CalculateValue::LINEAR(const std::vector<Keyframe<float>>& keyframes, float time) {
    ///===========================================
    /// 例外処理
    ///===========================================
    {
        assert(!keyframes.empty());
        if (keyframes.size() == 1 || time <= keyframes[0].time) {
            return keyframes[0].value;
        }
    }
    for (size_t index = 0; index < keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;
        // index と nextIndex の 2つを
        // 取得して 現時刻が 範囲内か
        if (keyframes[index].time <= time &&
            time <= keyframes[nextIndex].time) {
            // 範囲内 で 保管
            float t = (time - keyframes[index].time) /
                      (keyframes[nextIndex].time - keyframes[index].time);
            return std::lerp(keyframes[index].value, keyframes[nextIndex].value, t);
        }
    }
    // 登録されている時間より 後ろ
    // 最後の 値を返す
    return (*keyframes.rbegin()).value;
}
Vec3f CalculateValue::LINEAR(const std::vector<KeyframeVector3>& keyframes, float time) {
    ///===========================================
    /// 例外処理
    ///===========================================
    {
        assert(!keyframes.empty());
        if (keyframes.size() == 1 || time <= keyframes[0].time) {
            return keyframes[0].value;
        }
    }

    for (size_t index = 0; index < keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;

        // index と nextIndex の 2つを
        // 取得して 現時刻が 範囲内か
        if (keyframes[index].time <= time &&
            time <= keyframes[nextIndex].time) {
            // 範囲内 で 保管
            float t = (time - keyframes[index].time) /
                      (keyframes[nextIndex].time - keyframes[index].time);
            return Lerp(keyframes[index].value, keyframes[nextIndex].value, t);
        }
    }

    // 登録されている時間より 後ろ
    // 最後の 値を
    return (*keyframes.rbegin()).value;
}
Vec4f CalculateValue::LINEAR(const std::vector<Keyframe<Vec4f>>& keyframes, float time) {
    ///===========================================
    /// 例外処理
    ///===========================================
    {
        assert(!keyframes.empty());
        if (keyframes.size() == 1 || time <= keyframes[0].time) {
            return keyframes[0].value;
        }
    }
    for (size_t index = 0; index < keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;
        // index と nextIndex の 2つを
        // 取得して 現時刻が 範囲内か
        if (keyframes[index].time <= time &&
            time <= keyframes[nextIndex].time) {
            // 範囲内 で 保管
            float t = (time - keyframes[index].time) /
                      (keyframes[nextIndex].time - keyframes[index].time);
            return Lerp(keyframes[index].value, keyframes[nextIndex].value, t);
        }
    }
    // 登録されている時間より 後ろ
    // 最後の 値を返す
    return (*keyframes.rbegin()).value;
}
Quaternion CalculateValue::LINEAR(const std::vector<KeyframeQuaternion>& keyframes, float time) {
    ///===========================================
    /// 例外処理
    ///===========================================
    {
        assert(!keyframes.empty());
        if (keyframes.size() == 1 || time <= keyframes[0].time) {
            return keyframes[0].value;
        }
    }
    for (size_t index = 0; index < keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;

        // index と nextIndex の 2つを
        // 取得して 現時刻が 範囲内か
        if (keyframes[index].time <= time &&
            time <= keyframes[nextIndex].time) {
            // 範囲内 で 保管
            float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
            return Slerp(keyframes[index].value, keyframes[nextIndex].value, t);
        }
    }

    // 登録されている時間より 後ろ
    // 最後の 値を返す
    return (*keyframes.rbegin()).value;
}

float CalculateValue::Step(const std::vector<Keyframe<float>>& keyframes, float time) {
    ///===========================================
    /// 例外処理
    ///===========================================
    {
        assert(!keyframes.empty());
        if (keyframes.size() == 1 || time <= keyframes[0].time) {
            return keyframes[0].value;
        }
    }
    for (size_t index = 0; index < keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;
        // index と nextIndex の 2つを
        // 取得して 現時刻が 範囲内か
        if (keyframes[index].time <= time &&
            time <= keyframes[nextIndex].time) {
            // 範囲内 で 保管
            float t = (time - keyframes[index].time) /
                      (keyframes[nextIndex].time - keyframes[index].time);
            return std::lerp(keyframes[index].value, keyframes[nextIndex].value, t);
        }
    }
    // 登録されている時間より 後ろ
    // 最後の 値を返す
    return (*keyframes.rbegin()).value;
}
Vec3f CalculateValue::Step(const std::vector<KeyframeVector3>& keyframes, float time) {
    assert(!keyframes.empty());
    if (keyframes.size() == 1 || time <= keyframes[0].time) {
        return keyframes[0].value;
    }
    for (size_t index = 0; index < keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;
        if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
            return keyframes[index].value;
        }
    }
    return (*keyframes.rbegin()).value;
}
Vec4f CalculateValue::Step(const std::vector<Keyframe<Vec4f>>& keyframes, float time) {
    assert(!keyframes.empty());
    if (keyframes.size() == 1 || time <= keyframes[0].time) {
        return keyframes[0].value;
    }
    for (size_t index = 0; index < keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;
        if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
            return keyframes[index].value;
        }
    }
    return (*keyframes.rbegin()).value;
}
Quaternion CalculateValue::Step(const std::vector<KeyframeQuaternion>& keyframes, float time) {
    assert(!keyframes.empty());
    if (keyframes.size() == 1 || time <= keyframes[0].time) {
        return keyframes[0].value;
    }
    for (size_t index = 0; index < keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;
        if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
            return keyframes[index].value;
        }
    }
    return (*keyframes.rbegin()).value;
}

void Animation::ApplyAnimationToNodes(
    ModelNode& node,
    const Matrix4x4& parentTransform,
    const Animation* animation) {
    node.localMatrix          = animation->CalculateNodeLocal(node.name);
    Matrix4x4 globalTransform = parentTransform * node.localMatrix;

    // 子ノードに再帰的に適用
    for (auto& child : node.children) {
        ApplyAnimationToNodes(child, globalTransform, animation);
    }
}

Vec3f Animation::getCurrentScale(const std::string& nodeName) const {
    auto itr = data->nodeAnimations.find(nodeName);
    if (itr == data->nodeAnimations.end()) {
        return Vec3f(1.0f, 1.0f, 1.0f);
    }

    if (itr->second.interpolationType == InterpolationType::STEP) {
        return CalculateValue::Step(itr->second.scale, currentAnimationTime);
    }
    return CalculateValue::LINEAR(itr->second.scale, currentAnimationTime);
}

Quaternion Animation::getCurrentRotate(const std::string& nodeName) const {
    auto itr = data->nodeAnimations.find(nodeName);
    if (itr == data->nodeAnimations.end()) {
        return Quaternion::Identity();
    }

    if (itr->second.interpolationType == InterpolationType::STEP) {
        return CalculateValue::Step(itr->second.rotate, currentAnimationTime);
    }
    return CalculateValue::LINEAR(itr->second.rotate, currentAnimationTime);
}

Vec3f Animation::getCurrentTranslate(const std::string& nodeName) const {
    auto itr = data->nodeAnimations.find(nodeName);
    if (itr == data->nodeAnimations.end()) {
        return Vec3f(0.0f, 0.0f, 0.0f);
    }
    if (itr->second.interpolationType == InterpolationType::STEP) {
        return CalculateValue::Step(itr->second.translate, currentAnimationTime);
    }
    return CalculateValue::LINEAR(itr->second.translate, currentAnimationTime);
}

#include "Animation.h"

#include "model/Model.h"

void Animation::Update(float deltaTime, Model* model, const Matrix4x4& parentTransform) {
    {
        // 時間更新
        currentAnimationTime += deltaTime;

        // リピート
        if (currentAnimationTime > duration) {
            currentAnimationTime = std::fmod(currentAnimationTime, duration);
        }
    }

    {
        ApplyAnimationToNodes(model->meshData_->rootNode, MakeMatrix::Identity(), this);
    }
}

Matrix4x4 Animation::CalculateNodeLocal(const std::string& nodeName) const {
    auto it = data->nodeAnimations.find(nodeName);
    if (it == data->nodeAnimations.end()) {
        // ノードに対応するアニメーションがない場合、単位行列を返す
        return MakeMatrix::Identity();
    }

    const NodeAnimation& nodeAnimation = it->second;

    Vector3 scale     = CalculateValue(nodeAnimation.scale, currentAnimationTime);
    Quaternion rotate = Quaternion::Normalize(CalculateValue(nodeAnimation.rotate, currentAnimationTime));
    Vector3 translate = CalculateValue(nodeAnimation.translate, currentAnimationTime);
    return MakeMatrix::Affine(scale, rotate, translate);
}

Vector3 Animation::CalculateValue(const std::vector<KeyframeVector3>& keyframes, float time) const {
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

Quaternion Animation::CalculateValue(
    const std::vector<KeyframeQuaternion>& keyframes, float time) const {
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
            return Lerp(keyframes[index].value, keyframes[nextIndex].value, t);
        }
    }

    // 登録されている時間より 後ろ
    // 最後の 値を返す
    return (*keyframes.rbegin()).value;
}

void Animation::ApplyAnimationToNodes(ModelNode& node,const Matrix4x4& parentTransform,const Animation* animation){
    node.localMatrix          = animation->CalculateNodeLocal(node.name);
    Matrix4x4 globalTransform = parentTransform * node.localMatrix;

    // 子ノードに再帰的に適用
    for(auto& child : node.children){
        ApplyAnimationToNodes(child,globalTransform,animation);
    }
}

Vector3 Animation::getCurrentScale(const std::string& nodeName) const{
    auto itr = data->nodeAnimations.find(nodeName);
    if(itr == data->nodeAnimations.end()){
        return Vector3(1.0f,1.0f,1.0f);
    }
    return CalculateValue(itr->second.scale,currentAnimationTime);
}

Quaternion Animation::getCurrentRotate(const std::string& nodeName) const{
    auto itr = data->nodeAnimations.find(nodeName);
    if(itr == data->nodeAnimations.end()){
        return Quaternion::Identity();
    }
    return CalculateValue(itr->second.rotate,currentAnimationTime);
}

Vector3 Animation::getCurrentTranslate(const std::string& nodeName) const{
    auto itr = data->nodeAnimations.find(nodeName);
    if(itr == data->nodeAnimations.end()){
        return Vector3(0.0f,0.0f,0.0f);
    }
    return CalculateValue(itr->second.translate,currentAnimationTime);
}

#include "TextureEffectAnimation.h"

/// engine
#define ENGINE_ECS
#define DELTA_TIME
#include "EngineInclude.h"
// component
#include "component/effect/TextureEffectParam.h"

void TextureEffectAnimation::Initialize() {
}

void TextureEffectAnimation::Finalize() {
}

void TextureEffectAnimation::UpdateEntity(GameEntity* _entity) {
    auto* textureEffectParam = getComponent<TextureEffectParam>(_entity);

    if (!textureEffectParam) {
        return;
    }

    auto updateAnimation = [](UVTransform& _uvTrans,TextureEffectParam::UVAnimation& _uvAnim, AnimationState& _state) {
        if (_state.isPlay_) {
            _uvAnim.currentTime += getMainDeltaTime();

            // 時間が 再生時間を 過ぎたら
            if (_uvAnim.currentTime > _uvAnim.duration) {
                _state.isEnd_ = true;
                // ループする場合は時間を0に戻す
                if (_state.isLoop_) {
                    _uvAnim.currentTime = 0.0f;
                } else {
                    // ループしない場合は再生を終了
                    _state.isPlay_ = false;
                }
            }

            _uvTrans.scale_ = CalculateValue::Linear(
                _uvAnim.scale,
                _uvAnim.currentTime);

            _uvTrans.rotate_ = CalculateValue::Linear(
                _uvAnim.rotate,
                _uvAnim.currentTime);

            _uvTrans.translate_ = CalculateValue::Linear(
                _uvAnim.translate,
                _uvAnim.currentTime);
        }
    };

    // dissolveAnim
    updateAnimation(textureEffectParam->getEffectParamData().dissolveUV, textureEffectParam->getDissolveAnim(), textureEffectParam->getDissolveAnimState());
    // distortionAnim
    updateAnimation(textureEffectParam->getEffectParamData().distortionUV, textureEffectParam->getDistortionAnim(), textureEffectParam->getDistortionAnimState());
    // maskAnim
    updateAnimation(textureEffectParam->getEffectParamData().maskUV, textureEffectParam->getMaskAnim(), textureEffectParam->getMaskAnimState());

    textureEffectParam->getEffectParamData().UpdateTransform();
}

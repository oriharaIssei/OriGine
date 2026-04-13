#pragma once

#include "component/IComponent.h"

/// stl
#include <string>

/// math
#include <Quaternion.h>
#include <Vector3.h>

namespace OriGine {

// 前方宣言
struct Transform;

/// <summary>
/// 速度・加速度ベースで Transform を毎フレーム変化させるコンポーネント.
/// キーフレーム不要で、等速回転や加速移動などに使用する.
/// </summary>
class TransformRateAnimation
    : public IComponent {
    friend void to_json(nlohmann::json& _j, const TransformRateAnimation& _comp);
    friend void from_json(const nlohmann::json& _j, TransformRateAnimation& _comp);

public:
    /// <summary>
    /// 速度・加速度パラメータ（毎秒）
    /// </summary>
    struct RateParam {
        Vec3f velocity     = {0.0f, 0.0f, 0.0f}; // 毎秒の変化量
        Vec3f acceleration = {0.0f, 0.0f, 0.0f}; // 毎秒の速度変化量
    };

public:
    TransformRateAnimation();
    ~TransformRateAnimation() override;

    void Initialize(Scene* _scene, EntityHandle _entity) override;
    void Finalize() override;

    void Edit(Scene* _scene, EntityHandle _entity, const ::std::string& _parentLabel) override;

    /// <summary>
    /// 毎フレーム呼び出し. 速度・加速度に基づいて Transform を更新する.
    /// </summary>
    /// <param name="_deltaTime">デルタタイム（秒）</param>
    /// <param name="_transform">更新対象の Transform</param>
    void Update(float _deltaTime, Transform* _transform);

private:
    int32_t targetTransformIndex_ = -1;

    bool isPlay_ = false;

    /// rate-based animation (速度・加速度)
    RateParam scaleRate_;
    RateParam rotateRate_;     // 回転速度・加速度（ラジアン/秒, 各軸）
    RateParam translateRate_;

public: // アクセッサ
    bool IsPlaying() const { return isPlay_; }
    void SetIsPlay(bool _isPlay) { isPlay_ = _isPlay; }

    int32_t GetTargetTransformIndex() const { return targetTransformIndex_; }
    void SetTargetTransformIndex(int32_t _idx) { targetTransformIndex_ = _idx; }

    RateParam& GetScaleRate() { return scaleRate_; }
    RateParam& GetRotateRate() { return rotateRate_; }
    RateParam& GetTranslateRate() { return translateRate_; }

    const RateParam& GetScaleRate() const { return scaleRate_; }
    const RateParam& GetRotateRate() const { return rotateRate_; }
    const RateParam& GetTranslateRate() const { return translateRate_; }

    void SetScaleRate(const RateParam& _rate) { scaleRate_ = _rate; }
    void SetRotateRate(const RateParam& _rate) { rotateRate_ = _rate; }
    void SetTranslateRate(const RateParam& _rate) { translateRate_ = _rate; }
};

} // namespace OriGine

#pragma once

/// stl
#include <array>
#include <functional>
#include <string>
#include <unordered_map>

namespace OriGine {

/// <summary>
///  イージング方式
/// </summary>
enum class EaseType : int {
    Linear,
    EaseInSine,
    EaseOutSine,
    EaseInOutSine,

    EaseInQuad,
    EaseOutQuad,
    EaseInOutQuad,

    EaseInCubic,
    EaseOutCubic,
    EaseInOutCubic,

    EaseInQuart,
    EaseOutQuart,
    EaseInOutQuart,

    EaseInBack,
    EaseOutBack,
    EaseInOutBack,

    EaseInElastic,
    EaseOutElastic,
    EaseInOutElastic,

    EaseInBounce,
    EaseOutBounce,
    EaseInOutBounce,

    COUNT
};

/// <summary>
/// 線形補間
/// </summary>
/// <param name="_t">時間(0-1)</param>
/// <returns>補間値</returns>
float Linear(float _t);

/// <summary>
/// Sine In イージング
/// </summary>
float EaseInSine(float _time);

/// <summary>
/// Sine Out イージング
/// </summary>
float EaseOutSine(float _t);

/// <summary>
/// Sine InOut イージング
/// </summary>
float EaseInOutSine(float _t);

/// <summary>
/// Quad In イージング
/// </summary>
float EaseInQuad(float _t);

/// <summary>
/// Quad Out イージング
/// </summary>
float EaseOutQuad(float _t);

/// <summary>
/// Quad InOut イージング
/// </summary>
float EaseInOutQuad(float _t);

/// <summary>
/// Cubic In イージング
/// </summary>
float EaseInCubic(float _t);

/// <summary>
/// Cubic Out イージング
/// </summary>
float EaseOutCubic(float _t);

/// <summary>
/// Cubic InOut イージング
/// </summary>
float EaseInOutCubic(float _t);

/// <summary>
/// Quart In イージング
/// </summary>
float EaseInQuart(float _t);

/// <summary>
/// Quart Out イージング
/// </summary>
float EaseOutQuart(float _t);

/// <summary>
/// Quart InOut イージング
/// </summary>
float EaseInOutQuart(float _t);

/// <summary>
/// Back In イージング
/// </summary>
float EaseInBack(float _t);

/// <summary>
/// Back Out イージング
/// </summary>
float EaseOutBack(float _t);

/// <summary>
/// Back InOut イージング
/// </summary>
float EaseInOutBack(float _t);

/// <summary>
/// Elastic In イージング
/// </summary>
float EaseInElastic(float _t);

/// <summary>
/// Elastic Out イージング
/// </summary>
float EaseOutElastic(float _t);

/// <summary>
/// Elastic InOut イージング
/// </summary>
float EaseInOutElastic(float _t);

/// <summary>
/// Bounce In イージング
/// </summary>
float EaseInBounce(float _t);

/// <summary>
/// Bounce Out イージング
/// </summary>
float EaseOutBounce(float _t);

/// <summary>
/// Bounce InOut イージング
/// </summary>
float EaseInOutBounce(float _t);

static ::std::array<::std::function<float(float)>, static_cast<int>(EaseType::COUNT)> EasingFunctions = {
    Linear,

    EaseInSine,
    EaseOutSine,
    EaseInOutSine,

    EaseInQuad,
    EaseOutQuad,
    EaseInOutQuad,

    EaseInCubic,
    EaseOutCubic,
    EaseInOutCubic,

    EaseInQuart,
    EaseOutQuart,
    EaseInOutQuart,

    EaseInBack,
    EaseOutBack,
    EaseInOutBack,

    EaseInElastic,
    EaseOutElastic,
    EaseInOutElastic,

    EaseInBounce,
    EaseOutBounce,
    EaseInOutBounce,
};

static ::std::unordered_map<EaseType, ::std::string> EasingNames = {
    {EaseType::Linear, "Linear"},
    {EaseType::EaseInSine, "EaseInSine"},
    {EaseType::EaseOutSine, "EaseOutSine"},
    {EaseType::EaseInOutSine, "EaseInOutSine"},
    {EaseType::EaseInQuad, "EaseInQuad"},
    {EaseType::EaseOutQuad, "EaseOutQuad"},
    {EaseType::EaseInOutQuad, "EaseInOutQuad"},
    {EaseType::EaseInCubic, "EaseInCubic"},
    {EaseType::EaseOutCubic, "EaseOutCubic"},
    {EaseType::EaseInOutCubic, "EaseInOutCubic"},
    {EaseType::EaseInQuart, "EaseInQuart"},
    {EaseType::EaseOutQuart, "EaseOutQuart"},
    {EaseType::EaseInOutQuart, "EaseInOutQuart"},
    {EaseType::EaseInBack, "EaseInBack"},
    {EaseType::EaseOutBack, "EaseOutBack"},
    {EaseType::EaseInOutBack, "EaseInOutBack"},
    {EaseType::EaseInElastic, "EaseInElastic"},
    {EaseType::EaseOutElastic, "EaseOutElastic"},
    {EaseType::EaseInOutElastic, "EaseInOutElastic"},
    {EaseType::EaseInBounce, "EaseInBounce"},
    {EaseType::EaseOutBounce, "EaseOutBounce"},
    {EaseType::EaseInOutBounce, "EaseInOutBounce"},
};

#ifdef _DEBUG
/// <summary>
/// イージング設定用GUIを表示
/// </summary>
/// <param name="_label">ラベル</param>
/// <param name="_easeType">設定対象のイージングタイプ</param>
void EasingComboGui(const ::std::string& _label, EaseType& _easeType);
#endif // _DEBUG

} // namespace OriGine

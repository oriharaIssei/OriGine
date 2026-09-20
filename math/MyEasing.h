#pragma once

/// stl
#include <array>
#include <functional>
#include <string>
#include <unordered_map>

/// DLL境界
#include "OriGineApi.h"

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
/// <param name="t">時間(0-1)</param>
/// <returns>補間値</returns>
ORIGINE_API float Linear(float t);

/// <summary>
/// Sine In イージング
/// </summary>
ORIGINE_API float EaseInSine(float time);

/// <summary>
/// Sine Out イージング
/// </summary>
ORIGINE_API float EaseOutSine(float t);

/// <summary>
/// Sine InOut イージング
/// </summary>
ORIGINE_API float EaseInOutSine(float t);

/// <summary>
/// Quad In イージング
/// </summary>
ORIGINE_API float EaseInQuad(float t);

/// <summary>
/// Quad Out イージング
/// </summary>
ORIGINE_API float EaseOutQuad(float t);

/// <summary>
/// Quad InOut イージング
/// </summary>
ORIGINE_API float EaseInOutQuad(float t);

/// <summary>
/// Cubic In イージング
/// </summary>
ORIGINE_API float EaseInCubic(float t);

/// <summary>
/// Cubic Out イージング
/// </summary>
ORIGINE_API float EaseOutCubic(float t);

/// <summary>
/// Cubic InOut イージング
/// </summary>
ORIGINE_API float EaseInOutCubic(float t);

/// <summary>
/// Quart In イージング
/// </summary>
ORIGINE_API float EaseInQuart(float t);

/// <summary>
/// Quart Out イージング
/// </summary>
ORIGINE_API float EaseOutQuart(float t);

/// <summary>
/// Quart InOut イージング
/// </summary>
ORIGINE_API float EaseInOutQuart(float t);

/// <summary>
/// Back In イージング
/// </summary>
ORIGINE_API float EaseInBack(float t);

/// <summary>
/// Back Out イージング
/// </summary>
ORIGINE_API float EaseOutBack(float t);

/// <summary>
/// Back InOut イージング
/// </summary>
ORIGINE_API float EaseInOutBack(float t);

/// <summary>
/// Elastic In イージング
/// </summary>
ORIGINE_API float EaseInElastic(float t);

/// <summary>
/// Elastic Out イージング
/// </summary>
ORIGINE_API float EaseOutElastic(float t);

/// <summary>
/// Elastic InOut イージング
/// </summary>
ORIGINE_API float EaseInOutElastic(float t);

/// <summary>
/// Bounce In イージング
/// </summary>
ORIGINE_API float EaseInBounce(float t);

/// <summary>
/// Bounce Out イージング
/// </summary>
ORIGINE_API float EaseOutBounce(float t);

/// <summary>
/// Bounce InOut イージング
/// </summary>
ORIGINE_API float EaseInOutBounce(float t);

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

#ifdef ORIGINE_EDITOR_ENABLED
/// <summary>
/// イージング設定用GUIを表示
/// </summary>
/// <param name="label">ラベル</param>
/// <param name="easeType">設定対象のイージングタイプ</param>
ORIGINE_API void EasingComboGui(const ::std::string& label, EaseType& easeType);
#endif // _DEBUG

} // namespace OriGine

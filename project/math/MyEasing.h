#pragma once

#include <array>
#include <functional>
#include <unordered_map>

#include <cmath>
#include <numbers>
#include <string>

/// <summary>
///  イージング方式
/// </summary>
enum class EaseType : int{
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

constexpr float pi_float = static_cast<float>(std::numbers::pi);

float EaseInSine(float _time);

float EaseOutSine(float _t);

float EaseInOutSine(float _t);

float EaseInQuad(float _t);

float EaseOutQuad(float _t);


float EaseInOutQuad(float _t);

float EaseInCubic(float _t);

float EaseOutCubic(float _t);

float EaseInOutCubic(float _t);

float EaseInQuart(float _t);

float EaseOutQuart(float _t);

float EaseInOutQuart(float _t);

float EaseInBack(float _t);

float EaseOutBack(float _t);

float EaseInOutBack(float _t);

float EaseInElastic(float _t);

float EaseOutElastic(float _t);

float EaseInOutElastic(float _t);

float EaseInBounce(float _t);

float EaseOutBounce(float _t);

float EaseInOutBounce(float _t);


static std::array<std::function<float(float)>,static_cast<int>(EaseType::COUNT)> EasingFunctions_ = {
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

static std::unordered_map<EaseType,std::string> EasingNames = {
	{EaseType::EaseInSine,"EaseInSine"},
	{EaseType::EaseOutSine,"EaseOutSine"},
	{EaseType::EaseInOutSine,"EaseInOutSine"},
	{EaseType::EaseInQuad,"EaseInQuad"},
	{EaseType::EaseOutQuad,"EaseOutQuad"},
	{EaseType::EaseInOutQuad,"EaseInOutQuad"},
	{EaseType::EaseInCubic,"EaseInCubic"},
	{EaseType::EaseOutCubic,"EaseOutCubic"},
	{EaseType::EaseInOutCubic,"EaseInOutCubic"},
	{EaseType::EaseInQuart,"EaseInQuart"},
	{EaseType::EaseOutQuart,"EaseOutQuart"},
	{EaseType::EaseInOutQuart,"EaseInOutQuart"},
	{EaseType::EaseInBack,"EaseInBack"},
	{EaseType::EaseOutBack,"EaseOutBack"},
	{EaseType::EaseInOutBack,"EaseInOutBack"},
	{EaseType::EaseInElastic,"EaseInElastic"},
	{EaseType::EaseOutElastic,"EaseOutElastic"},
	{EaseType::EaseInOutElastic,"EaseInOutElastic"},
	{EaseType::EaseInBounce,"EaseInBounce"},
	{EaseType::EaseOutBounce,"EaseOutBounce"},
	{EaseType::EaseInOutBounce,"EaseInOutBounce"},
};

#include "Easing.h"
#define _USE_MATH_DEFINES
#include <math.h>

#define float_cast static_cast<float>

float EasingLerp(std::function<float(float)> easingFunc, float t, float start, float end) {
	return Lerp(easingFunc(t), start, end);
}

Vector2 EasingLerp(std::function<float(float)> easingFunc, float t, const Vector2& start, const Vector2& end) {
	return Lerp(easingFunc(t), start, end);
}

float Easing::easeInSine(float x) {
	return 1.0f - cosf((x * float_cast(M_PI)) / 2.0f);
}

float Easing::easeOutSine(float x) {
	return sinf((x * float_cast(M_PI)) / 2);
}

float Easing::easeInOutSine(float x) {
	return -(cosf(float_cast(M_PI) * x) - 1) / 2;
}

float Easing::easeInCubic(float x) {
	return x * x * x;
}

float Easing::easeOutCubic(float x) {
	return 1.0f - powf(1 - x, 3);
}

float Easing::easeInQuart(float x) {
	return x * x * x * x;
}

float Easing::easeOutQuart(float x) {
	return 1 - powf(1.0f - x, 4);
}

float Easing::easeInOutQuart(float x) {
	return x < 0.5 ? 8 * x * x * x * x : 1 - powf(-2 * x + 2, 4) / 2;
}
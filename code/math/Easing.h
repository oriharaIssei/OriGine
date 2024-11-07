#pragma once

#include "Vector2.h"
#include "Vector3.h"

#include <functional>

template <typename T>
T Lerp(const float &t,const T &start,const T &end);

Vector2 Lerp(float t,const Vector2 &startPos,const Vector2 &endPos);
Vector3 Lerp(float t,const Vector3 &start,const Vector3 &end);

template <typename T>
T EasingLerp(std::function<float(float)> easingFunc,float t,T start,T end);

Vector2 EasingLerp(std::function<float(float)> easingFunc,float t,const Vector2 &start,const Vector2 &end);

namespace Easing {
	float easeInSine(float x);
	float easeOutSine(float x);
	float easeInOutSine(float x);

	float easeInCubic(float x);
	float easeOutCubic(float x);

	float easeInQuart(float x);
	float easeOutQuart(float x);
	float easeInOutQuart(float x);
}

template<typename T>
inline T Lerp(const float &t,const T &start,const T &end) {
	return static_cast<T>(((1.0 - t) * static_cast<float>(start)) + (t * static_cast<float>(end)));
}

inline Vector2 Lerp(float t,const Vector2 &startPos,const Vector2 &endPos) {
	Vector2 lerp;
	lerp.x = Lerp<float>(t,startPos.x,endPos.x);
	lerp.y = Lerp<float>(t,startPos.y,endPos.y);

	return lerp;
}

inline Vector3 Lerp(float t,const Vector3 &start,const Vector3 &end) {
	Vector3 lerp;
	lerp.x = Lerp<float>(t,start.x,end.x);
	lerp.y = Lerp<float>(t,start.y,end.y);
	lerp.z = Lerp<float>(t,start.z,end.z);

	return lerp;
}

template<typename T>
inline T EasingLerp(std::function<float(float)> easingFunc,float t,T start,T end) {
	float easedT = easingFunc(t);
	easedT = std::min<float>(easedT,1.0f);
	return Lerp(easedT,start,end);
}

template<typename T>
inline Vector2 EasingLerp(std::function<float(float)> easingFunc,float t,const Vector2 &start,const Vector2 &end) {
	float easedT = easingFunc(t);
	return Lerp(easedT,start,end);
}
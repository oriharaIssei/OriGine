#pragma once

#include <Array>
#include <string>

#include "Vector3.h"

enum class EmitterShapeType : int32_t{
	SPHERE,
	AABB,
	Count // 使わない
};

static const int32_t shapeTypeCount = static_cast<int32_t>(EmitterShapeType::Count);

static std::array<std::string,shapeTypeCount> emitterShapeTypeWord_ = {
	"Sphere",
	"AABB"
};

struct EmitterShape{
	EmitterShape(EmitterShapeType type):type_(type){}

#ifdef _DEBUG
	virtual void Debug() = 0;
#endif // _DEBUG

	virtual Vector3 getSpawnPos() = 0;
private:
	const EmitterShapeType type_;
};
struct EmitterSphere
	:EmitterShape{
	EmitterSphere():EmitterShape(EmitterShapeType::SPHERE){}
	EmitterSphere(float radius):
		EmitterShape(EmitterShapeType::SPHERE),
		radius_(radius){}

	float radius_ = 0.0f;

#ifdef _DEBUG
	void Debug()override;
#endif // _DEBUG
	Vector3 getSpawnPos()override;
};
struct EmitterAABB
	:EmitterShape{
	EmitterAABB():EmitterShape(EmitterShapeType::AABB){}
	EmitterAABB(const Vector3& min,const Vector3& max)
		:EmitterShape(EmitterShapeType::AABB),
		min_(min),max_(max){}

	Vector3 min_ = {0.0f,0.0f,0.0f};
	Vector3 max_ = {0.0f,0.0f,0.0f};

#ifdef _DEBUG
	void Debug()override;
#endif // _DEBUG
	Vector3 getSpawnPos()override;
};
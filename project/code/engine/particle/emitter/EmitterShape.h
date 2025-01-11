#pragma once

#include <Array>
#include <string>

#include "globalVariables/SerializedField.h"
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

	virtual Vec3f getSpawnPos() = 0;
private:
	const EmitterShapeType type_;
};
struct EmitterSphere
	:EmitterShape{
	EmitterSphere(const std::string& scene,
				  const std::string& emitterName)
		:EmitterShape(EmitterShapeType::SPHERE),
		radius_{scene,emitterName,"spawnSphereRadius"}{}

	SerializedField<float> radius_;

#ifdef _DEBUG
	void Debug()override;
#endif // _DEBUG
	Vec3f getSpawnPos()override;
};
struct EmitterAABB
	:EmitterShape{
	EmitterAABB(const std::string& scene,
				const std::string& emitterName)
		:EmitterShape(EmitterShapeType::AABB),
		min_{scene,emitterName,"spawnAABBmin"},
		max_{scene,emitterName,"spawnAABBmax"}{}

	SerializedField<Vec3f> min_;
	SerializedField<Vec3f> max_;

#ifdef _DEBUG
	void Debug()override;
#endif // _DEBUG
	Vec3f getSpawnPos()override;
};
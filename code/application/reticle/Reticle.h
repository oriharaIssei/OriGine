#pragma once

#include <memory>

#include "sprite/Sprite.h"

#include "Matrix4x4.h"
#include "Vector2.h"

class RailCamera;
class Input;

class Reticle{
public:
	void Init();
	void Update(const RailCamera* camera,Input* input);
	void DrawSprite();

	void ResteStatus(){
		reticleLocal3dPos_ = {0.0f,0.0f,0.0f};
	}
private:
#ifdef _DEBUG
	Vector2 size_;
#endif // _DEBUG
	std::unique_ptr<Sprite> sprite_;

	Matrix4x4 viewPortMat_;

	Vector2 minPos_;
	Vector2 maxPos_;
	float speed_;

	Vector3 reticleLocal3dPos_;
	Vector3 reticleWorld3dPos_;
	Vector3 reticleScreenPos_;
	float kDistance3DReticle_;
public:
	const Vector3& getWorldPos()const{ return reticleWorld3dPos_; }

};
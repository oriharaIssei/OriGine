#pragma once

#include <memory>

#include "Transform/Transform.h"

class IPlayerBehavior;
class Object3d;

class Player{
public:
	Player();
	~Player();

	void Init();
	void Update();
	void Draw();
private:
	std::unique_ptr<IPlayerBehavior> currentBehavior_;

	std::unique_ptr<Object3d> drawObject3d_;
	Transform transform_;
public:
	void ChangeBehavior(IPlayerBehavior* next);

	const Vector3& getScale()const;
	const Quaternion& getRotate()const;
	const Vector3& getTranslate()const;

	void setScale(const Vector3& s);
	void setRotate(const Quaternion& q);
	void setTranslate(const Vector3& t);
};


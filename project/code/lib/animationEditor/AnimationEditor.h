#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Quaternion.h"
#include "Vector3.h"

struct Model;
struct Animation;

class AnimationEditor{
public:
	AnimationEditor();
	~AnimationEditor();

	void Init();
	void Update();
private:

	std::unique_ptr<Model> editAnimationModel_;
	std::unique_ptr<Animation> editAnimation_;
};
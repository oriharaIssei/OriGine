#pragma once

#include "FrameWork.h"

class Engine;
class GlobalVariables;
class SceneManager;

class MyGame
 :public FrameWork{
public:
	MyGame();
	~MyGame();

	void Init()override;
	void Finalize()override;

	void Run();

private:
	Engine* engine_ = nullptr;
	GlobalVariables* variables_ = nullptr;
	SceneManager* sceneManager_ = nullptr;
};
#pragma once

#include <list>
#include <memory>

#include <string>

#include "IScene.h"

class Input;
class MaterialManager;
class DxRtvArray;
class DxSrvArray;
class ParticleManager;
class DebugCamera;

struct Matrix4x4;
struct Vector3;

class GameScene
	:public IScene{
public:
	GameScene();
	~GameScene();

	void Init();
	void Update();

	void Draw3d();
	void DrawLine();
	void DrawSprite();
	void DrawParticle();
private:
#ifdef _DEBUG
	//DebugObjects
	std::unique_ptr<DebugCamera> debugCamera_;
#endif // _DEBUG

	Input* input_;
	MaterialManager* materialManager_;
};
#pragma once

#include <list>
#include <memory>

#include <string>

#include "audio/Audio.h"
#include "debugCamera/DebugCamera.h"
#include "transform/CameraTransform.h"

#include "engine/texture/RenderTexture.h"
#include "gameObject/IGameObject.h"

#include "particle/Emitter.h"

class Input;
class MaterialManager;
class DxRtvArray;
class DxSrvArray;

struct Matrix4x4;
struct Vector3;

class GameScene{
public:
	GameScene() = default;
	~GameScene();

	void Init();
	void Update();
	void Draw();
private:
	std::unique_ptr<DebugCamera> debugCamera_;
	IConstantBuffer<CameraTransform> cameraBuff_;
	Input* input_;

	std::shared_ptr<DxRtvArray> sceneRtvArray_;
	std::shared_ptr<DxSrvArray> sceneSrvArray_;
	std::unique_ptr<RenderTexture> sceneView_;

	std::list<std::unique_ptr<IGameObject>> gameObjects_;
	std::list<std::pair<std::string,std::string>> textureList_;
	std::list<std::pair<std::string,std::string>> objectList_;

	Emitter emitter_;

	MaterialManager* materialManager_;
};
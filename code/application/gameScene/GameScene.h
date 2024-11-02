#pragma once

#include <list>
#include <memory>

#include <string>

#include "audio/Audio.h"
#include "debugCamera/DebugCamera.h"
#include "engine/texture/RenderTexture.h"
#include "object3d/Object3d.h"
#include "transform/CameraTransform.h"

#include "../railCamera/RailCamera.h"
#include "RailEditor/RailEditor.h"

#include "../Beam.h"

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
	Input* input_;
#ifdef _DEBUG
	std::unique_ptr<DebugCamera> debugCamera_;
	bool isDebugCamera_ = true;
#endif // _DEBUG
	IConstantBuffer<CameraTransform> cameraBuff_;

	MaterialManager* materialManager_;

	std::shared_ptr<DxRtvArray> sceneRtvArray_;
	std::shared_ptr<DxSrvArray> sceneSrvArray_;
	std::unique_ptr<RenderTexture> sceneView_;

	std::unique_ptr<RailEditor> railEditor_;
	std::unique_ptr<RailCamera> railCamera_;

	std::list<std::pair<std::string,std::string>> textureList_;
	std::list<std::pair<std::string,std::string>> objectList_;

	Beam beam_;
};
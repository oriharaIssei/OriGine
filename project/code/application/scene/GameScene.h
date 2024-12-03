#pragma once

#include <list>
#include <memory>

#include <string>

#include "audio/Audio.h"
#include "engine/camera/debugCamera/DebugCamera.h"
#include "engine/directX12/RenderTexture.h"
#include "engine/particle/manager/ParticleManager.h"
#include "object3d/Object3d.h"
#include "transform/CameraTransform.h"

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
#ifdef _DEBUG
	std::unique_ptr<DebugCamera> debugCamera_;
#endif // _DEBUG

	IConstantBuffer<CameraTransform> cameraBuff_;
	Input* input_;
	MaterialManager* materialManager_;

	std::shared_ptr<DxRtvArray> sceneRtvArray_;
	std::shared_ptr<DxSrvArray> sceneSrvArray_;
	std::unique_ptr<RenderTexture> sceneView_;

	std::unique_ptr<Object3d> object_;

	ParticleManager* particleManager;
};
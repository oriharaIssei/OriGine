#pragma once

#include <list>
#include <memory>

#include <string>

#include "audio/Audio.h"
#include "engine/texture/RenderTexture.h"
#include "object3d/Object3d.h"
#include "transform/CameraTransform.h"

#include "../Beam/Beam.h"
#include "../collisionManager/CollisionManager.h"
#include "../railCamera/RailCamera.h"
#include "../RailEditor/RailEditor.h"
#include "../reticle/Reticle.h"
#include "code/application/Enemy/EnemyManager.h"
#include "Spline.h"

class Input;
class MaterialManager;
class DxRtvArray;
class DxSrvArray;

class Spline;
class Score;

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
	void TitleUpdate();
	void TitleDraw();
	void GameUpdate();
	void GameDraw();
	void GameClearUpdate();
	void GameClearDraw();

private:
	std::function<void()> currentUpdate_;
	std::function<void()> currentDraw_;

	Input* input_;

	IConstantBuffer<CameraTransform> cameraBuff_;

	MaterialManager* materialManager_;

	std::shared_ptr<DxRtvArray> sceneRtvArray_;
	std::shared_ptr<DxSrvArray> sceneSrvArray_;
	std::unique_ptr<RenderTexture> sceneView_;

#pragma region"Title"
	std::unique_ptr<Sprite> titleBackground_;
	std::unique_ptr<Sprite> titleBar_;
	std::unique_ptr<Sprite> howToStartGame_;
#pragma endregion

#pragma region"Game"
	std::unique_ptr<RailEditor> railEditor_;
	std::unique_ptr<Spline> spline_;
	std::unique_ptr<RailCamera> railCamera_;

	std::unique_ptr<EnemyManager> enemyManager_;

	std::unique_ptr<Reticle> reticle_;
	std::unique_ptr<Beam> beam_;
	std::unique_ptr<CollisionManager> collisionManager_;

	std::unique_ptr<Object3d> skyDome_;
	Score* score_;
#pragma endregion

#pragma region"GameClear"

#pragma endregion

};
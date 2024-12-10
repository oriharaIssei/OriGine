#include "GameScene.h"

#include <string>

#include "Engine.h"
#include "model/ModelManager.h"

#include "camera/Camera.h"
#include "directX12/DxCommand.h"
#include "directX12/DxRtvArray.h"
#include "directX12/DxRtvArrayManager.h"
#include "directX12/DxSrvArrayManager.h"
#include "directX12/RenderTexture.h"
#include "material/texture/TextureManager.h"
#include "myFileSystem/MyFileSystem.h"
#include "particle/manager/ParticleManager.h"
#include "primitiveDrawer/PrimitiveDrawer.h"
#include "sprite/SpriteCommon.h"

#ifdef _DEBUG
#include "camera/debugCamera/DebugCamera.h"
#include "imgui/imgui.h"
#endif // _DEBUG

GameScene::GameScene():IScene("GameScene"){}

GameScene::~GameScene(){}

void GameScene::Init(){
#ifdef _DEBUG
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Init();

	debugCamera_->setViewTranslate({0.0f,0.0f,-12.0f});
#endif // _DEBUG

	input_ = Input::getInstance();

	materialManager_ = Engine::getInstance()->getMaterialManager();

	object_.reset(Object3d::Create("resource/Models","cube.gltf"));
	object_->transform_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
	object_->transform_.openData_.UpdateMatrix();

	animation_ = ModelManager::LoadAnimation("resource/Models/AnimatedCube","AnimatedCube.gltf");
}

void GameScene::Update(){
#ifdef _DEBUG
	debugCamera_->Update();
	debugCamera_->DebugUpdate();
	Camera::getInstance()->setTransform(debugCamera_->getCameraTransform());
#endif // _DEBUG

	// model を animation で 動かす
	{
		animation_.UpdateTime(Engine::getInstance()->getDeltaTime());
		object_->transform_.openData_.worldMat =
			animation_.CalculateCurrentLocal();
		object_->transform_.ConvertToBuffer();
	}

#ifdef _DEBUG
	materialManager_->DebugUpdate();
#endif // _DEBUG

	Engine::getInstance()->getLightManager()->Update();
}

void GameScene::Draw3d(){
	object_->Draw();
}

void GameScene::DrawLine(){}

void GameScene::DrawSprite(){}

void GameScene::DrawParticle(){}
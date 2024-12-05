#include "GameScene.h"

#include "engine/camera/debugCamera/debugCamera.h"

#include <string>

#include "Engine.h"

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

	cameraBuff_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());

	input_ = Input::getInstance();

	materialManager_ = Engine::getInstance()->getMaterialManager();

	object_.reset(Object3d::Create("resource/Models","Enemy.obj"));
	object_->transform_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
	object_->transform_.openData_.UpdateMatrix();
	object_->transform_.ConvertToBuffer();

	particleManager = ParticleManager::getInstance();
}

void GameScene::Update(){
#ifdef _DEBUG
	debugCamera_->Update();
	debugCamera_->DebugUpdate();
	cameraBuff_.openData_.viewMat = debugCamera_->getCameraTransform().viewMat;
	cameraBuff_.openData_.projectionMat = debugCamera_->getCameraTransform().projectionMat;
	cameraBuff_.ConvertToBuffer();
#endif // _DEBUG

#ifdef _DEBUG
	particleManager->Edit();
	materialManager_->DebugUpdate();
#endif // _DEBUG

	Engine::getInstance()->getLightManager()->Update();
}

void GameScene::Draw3d(){
}

void GameScene::DrawLine(){}

void GameScene::DrawSprite(){}

void GameScene::DrawParticle(){
	particleManager->DrawDebug(cameraBuff_);
}
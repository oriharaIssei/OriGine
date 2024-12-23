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
#include "object3d/AnimationObject3d.h"
#include "particle/manager/ParticleManager.h"
#include "primitiveDrawer/PrimitiveDrawer.h"
#include "sprite/SpriteCommon.h"

#include "animationEditor/AnimationEditor.h"

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

    animationEditor_ = std::make_unique<AnimationEditor>();
    animationEditor_->Init();
}

void GameScene::Update(){
#ifdef _DEBUG
	debugCamera_->Update();
	debugCamera_->DebugUpdate();
	Camera::getInstance()->setTransform(debugCamera_->getCameraTransform());
#endif // _DEBUG

    animationEditor_->Update();

#ifdef _DEBUG
	materialManager_->DebugUpdate();
#endif // _DEBUG

	Engine::getInstance()->getLightManager()->Update();
}

void GameScene::Draw3d(){
    animationEditor_->DrawEditObject(); 
}

void GameScene::DrawLine(){}

void GameScene::DrawSprite(){}

void GameScene::DrawParticle(){}

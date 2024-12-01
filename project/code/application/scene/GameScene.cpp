#include "GameScene.h"

#include "engine/camera/debugCamera/debugCamera.h"

#include <string>

#include "Engine.h"

#include "directX12/DxCommand.h"
#include "directX12/DxRtvArray.h"
#include "directX12/DxRtvArrayManager.h"
#include "directX12/DxSrvArrayManager.h"
#include "material/texture/TextureManager.h"
#include "myFileSystem/MyFileSystem.h"
#include "primitiveDrawer/PrimitiveDrawer.h"
#include "sprite/SpriteCommon.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

constexpr char dockingIDName[] = "ObjectsWindow";

GameScene::~GameScene(){}

void GameScene::Init(){
#ifdef _DEBUG
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Init();

	debugCamera_->setViewTranslate({0.0f,0.0f,-12.0f});
#endif // _DEBUG

	cameraBuff_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());

	input_ = Input::getInstance();

	sceneRtvArray_ = DxRtvArrayManager::getInstance()->Create(1);
	sceneSrvArray_ = DxSrvArrayManager::getInstance()->Create(1);

	sceneView_ = std::make_unique<RenderTexture>(Engine::getInstance()->getDxCommand(),sceneRtvArray_.get(),sceneSrvArray_.get());
	sceneView_->Init({1280.0f,720.0f},DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,{0.0f,0.0f,0.0f,0.0f});

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
	materialManager_->DebugUpdate();
#endif // _DEBUG
}

void GameScene::Draw(){
	Engine::getInstance()->getLightManager()->Update();

	sceneView_->PreDraw();
	///===============================================
	/// 3d Object
	///===============================================
	Object3d::PreDraw();
	object_->Draw(cameraBuff_);

	///===============================================
	/// sprite
	///===============================================
	SpriteCommon::getInstance()->PreDraw();


	sceneView_->PostDraw();
	///===============================================
	/// off screen Rendering
	///===============================================
	Engine::getInstance()->ScreenPreDraw();
	sceneView_->DrawTexture();
	Engine::getInstance()->ScreenPostDraw();
}
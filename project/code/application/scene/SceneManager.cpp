#include "SceneManager.h"

#include "camera/Camera.h"
#include "engine/directX12/DxRtvArrayManager.h"
#include "engine/directX12/DxSrvArrayManager.h"
#include "engine/directX12/RenderTexture.h"
#include "IScene.h"
#include "particle/manager/ParticleManager.h"
#include "primitiveDrawer/PrimitiveDrawer.h"
#include "sprite/SpriteCommon.h"

#include "Engine.h"

SceneManager* SceneManager::getInstance(){
	static SceneManager instance;
	return &instance;
}

void SceneManager::Init(){
	sceneViewRtvArray_ = DxRtvArrayManager::getInstance()->Create(1);
	sceneViewSrvArray_ = DxSrvArrayManager::getInstance()->Create(1);

	sceneView_ = std::make_unique<RenderTexture>(Engine::getInstance()->getDxCommand(),sceneViewRtvArray_.get(),sceneViewSrvArray_.get());
	/// TODO
	// fix MagicNumber
	sceneView_->Init({1280.0f,720.0f},DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,{0.0f,0.0f,0.0f,0.0f});
}

void SceneManager::Finalize(){
	sceneView_->Finalize();
	sceneViewRtvArray_->Finalize();
	sceneViewSrvArray_->Finalize();
	currentScene_.reset();
}

void SceneManager::Update(){
	currentScene_->Update();

	Camera::getInstance()->DataConvertToBuffer();
}

void SceneManager::Draw(){
	sceneView_->PreDraw();
	///===============================================
	/// 3d Object
	///===============================================
	Object3d::PreDraw();

	currentScene_->Draw3d();
	///===============================================
	/// Line
	///===============================================
	PrimitiveDrawer::PreDrawLine();

	currentScene_->DrawLine();
	///===============================================
	/// Particle
	///===============================================
	ParticleManager::getInstance()->PreDraw();

	currentScene_->DrawParticle();
	///===============================================
	/// sprite
	///===============================================
	SpriteCommon::getInstance()->PreDraw();

	currentScene_->DrawSprite();
	///===============================================
	/// off screen Rendering
	///===============================================
	sceneView_->PostDraw();
	Engine::getInstance()->ScreenPreDraw();
	sceneView_->DrawTexture();
	Engine::getInstance()->ScreenPostDraw();
}

SceneManager::SceneManager(){}

SceneManager::~SceneManager(){}

void SceneManager::ChangeScene(std::unique_ptr<IScene> next){
	currentScene_ = std::move(next);
	currentScene_->Init();
}
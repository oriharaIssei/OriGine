#include "GameScene.h"

#include "debugCamera/debugCamera.h"

#include <string>

#include "../score/Score.h"
#include "directX12/DxCommand.h"
#include "directX12/DxRtvArray.h"
#include "directX12/DxRtvArrayManager.h"
#include "directX12/DxSrvArrayManager.h"
#include "material/TextureManager.h"
#include "myFileSystem/MyFileSystem.h"
#include "primitiveDrawer/PrimitiveDrawer.h"
#include "sprite/SpriteCommon.h"
#include "System.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

constexpr char dockingIDName[] = "ObjectsWindow";

GameScene::~GameScene(){
	score_->Finalize();
}

void GameScene::Init(){
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Init();

	debugCamera_->setViewTranslate({0.0f,0.0f,-12.0f});

	cameraBuff_.CreateBuffer(System::getInstance()->getDxDevice()->getDevice());

	input_ = Input::getInstance();

	sceneRtvArray_ = DxRtvArrayManager::getInstance()->Create(1);
	sceneSrvArray_ = DxSrvArrayManager::getInstance()->Create(1);

	sceneView_ = std::make_unique<RenderTexture>(System::getInstance()->getDxCommand(),sceneRtvArray_.get(),sceneSrvArray_.get());
	sceneView_->Init({1280.0f,720.0f},DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,{0.0f,0.0f,0.0f,0.0f});

	materialManager_ = System::getInstance()->getMaterialManager();

	railEditor_ = std::make_unique<RailEditor>(cameraBuff_.openData_);
	railEditor_->Init();

	spline_ = std::make_unique<Spline>(railEditor_->getControlPointPositions());

	railCamera_ = std::make_unique<RailCamera>();
	railCamera_->Init(railEditor_->getSegmentCount());
	railCamera_->SetSpline(spline_.get());

	enemyManager_ = std::make_unique<EnemyManager>();
	enemyManager_->Init();

	beam_ = std::make_unique<Beam>();
	beam_->Initialize();

	reticle_ = std::make_unique<Reticle>();
	reticle_->Init();

	collisionManager_ = std::make_unique<CollisionManager>();

	score_ = Score::getInstance();
	score_->Init();
}

void GameScene::Update(){
#ifdef _DEBUG
	if(input_->isTriggerKey(DIK_F1)){
		isDebugCamera_ = !isDebugCamera_;
	}

	if(isDebugCamera_){
		debugCamera_->Update();
		debugCamera_->DebugUpdate();
		cameraBuff_.openData_.viewMat = debugCamera_->getCameraTransform().viewMat;
		cameraBuff_.openData_.projectionMat = debugCamera_->getCameraTransform().projectionMat;
	} else{
		railCamera_->Update();
		cameraBuff_.openData_.viewMat = railCamera_->getCameraBuffer().viewMat;
		cameraBuff_.openData_.projectionMat = railCamera_->getCameraBuffer().projectionMat;
	}
#else
	railCamera_->Update();
	cameraBuff_.openData_.viewMat = railCamera_->getCameraBuffer().viewMat;
	cameraBuff_.openData_.projectionMat = railCamera_->getCameraBuffer().projectionMat;
#endif // _DEBUG

	cameraBuff_.ConvertToBuffer();

	railEditor_->Update();

	reticle_->Update(railCamera_.get(),input_);
	beam_->Update(railCamera_.get(),reticle_.get(),input_);

	enemyManager_->Update(railCamera_->GetCurrentDistance());

	collisionManager_->Update(enemyManager_.get(),beam_.get());

	score_->Update();

#ifdef _DEBUG
	ImGui::Begin("Materials");
	materialManager_->DebugUpdate();
	ImGui::End();
#endif // _DEBUG
}

void GameScene::Draw(){
	System::getInstance()->getDirectionalLight()->openData_.DebugUpdate();
	System::getInstance()->getPointLight()->openData_.DebugUpdate();
	System::getInstance()->getSpotLight()->openData_.DebugUpdate();

	sceneView_->PreDraw();

	///===============================================
	/// 3d Object
	///===============================================
	Object3d::PreDraw();
	railCamera_->Draw(cameraBuff_);
	railEditor_->Draw(cameraBuff_);

	beam_->Draw(cameraBuff_);

	enemyManager_->Draw(cameraBuff_);

	///===============================================
	/// sprite
	///===============================================
	SpriteCommon::getInstance()->PreDraw();
	reticle_->DrawSprite();
	score_->Draw();

	sceneView_->PostDraw();
	///===============================================
	/// off screen Rendering
	///===============================================
	System::getInstance()->ScreenPreDraw();
	sceneView_->DrawTexture();
	System::getInstance()->ScreenPostDraw();
}
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
#include "object3d/ModelManager.h"
#include "primitiveDrawer/PrimitiveDrawer.h"
#include "sprite/SpriteCommon.h"
#include "System.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

GameScene::~GameScene(){
	score_->Finalize();
}

void GameScene::Init(){
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

	skyDome_ = std::make_unique<Object3d>();
	skyDome_->SetModel(ModelManager::getInstance()->Create("resource/Models","Skydome.obj"));
	skyDome_->transform_.CreateBuffer(System::getInstance()->getDxDevice()->getDevice());
	skyDome_->transform_.openData_.UpdateMatrix();
	skyDome_->transform_.ConvertToBuffer();

	score_ = Score::getInstance();
	score_->Init();

	titleBackground_ = std::make_unique<Sprite>(SpriteCommon::getInstance());
	titleBackground_->Init("resource/white1x1.png");
	titleBackground_->setSize({1280.0f,720.0f});
	titleBackground_->setColor({0.0f,0.0f,0.0f,1.0f});

	howToStartGame_ = std::make_unique<Sprite>(SpriteCommon::getInstance());
	howToStartGame_->Init("resource/Texture/howToStart.png");
	howToStartGame_->setTextureSize({256.0f,36.0f});
	howToStartGame_->setSize({256.0f,36.0f});
	howToStartGame_->setAnchorPoint({0.5f,0.5f});
	howToStartGame_->setPosition({640.0f,420.0f});


	currentUpdate_ = [this](){TitleUpdate(); };
	currentDraw_ = [this](){TitleDraw(); };
}

void GameScene::Update(){
	currentUpdate_();

#ifdef _DEBUG
	ImGui::Begin("Materials");
	materialManager_->DebugUpdate();
	ImGui::End();
#endif // _DEBUG
}

void GameScene::Draw(){
	currentDraw_();
}

#pragma region"Title"
void GameScene::TitleUpdate(){

	if(input_->isReleaseKey(DIK_SPACE)){
		currentUpdate_ = [this](){GameUpdate(); };
		currentDraw_ = [this](){GameDraw(); };
	}
}

void GameScene::TitleDraw(){
	System::getInstance()->getLightManager()->Update();

	sceneView_->PreDraw();
	///===============================================
	/// 3d Object
	///===============================================
	Object3d::PreDraw();

	///===============================================
	/// sprite
	///===============================================
	SpriteCommon::getInstance()->PreDraw();

	titleBackground_->Draw();
	howToStartGame_->Draw();

	sceneView_->PostDraw();
	///===============================================
	/// off screen Rendering
	///===============================================
	System::getInstance()->ScreenPreDraw();
	sceneView_->DrawTexture();
	System::getInstance()->ScreenPostDraw();
}
#pragma endregion"Title"

#pragma region"Game"
void GameScene::GameUpdate(){
	railCamera_->Update();
	cameraBuff_.openData_.viewMat = railCamera_->getCameraBuffer().viewMat;
	cameraBuff_.openData_.projectionMat = railCamera_->getCameraBuffer().projectionMat;

	cameraBuff_.ConvertToBuffer();

	railEditor_->Update();

	reticle_->Update(railCamera_.get(),input_);
	beam_->Update(railCamera_.get(),reticle_.get(),input_);

	enemyManager_->Update(railCamera_->GetCurrentDistance());

	collisionManager_->Update(enemyManager_.get(),beam_.get(),reticle_.get(),railCamera_.get());

	score_->Update();

	if(spline_->GetTotalLength() - railCamera_->GetCurrentDistance() <= 10.0f){
		score_->InitOnGameClear();
		currentUpdate_ = [this](){GameClearUpdate(); };
		currentDraw_ = [this](){GameClearDraw(); };
	}
}

void GameScene::GameDraw(){
	System::getInstance()->getLightManager()->Update();

	sceneView_->PreDraw();
	///===============================================
	/// 3d Object
	///===============================================
	Object3d::PreDraw();
	skyDome_->Draw(cameraBuff_);

	railEditor_->Draw(cameraBuff_);

	beam_->Draw(cameraBuff_);

	enemyManager_->Draw(cameraBuff_);

	///===============================================
	/// sprite
	///===============================================
	SpriteCommon::getInstance()->PreDraw();
	reticle_->DrawSprite();
	score_->Draw();
	beam_->DrawSprite();

	sceneView_->PostDraw();
	///===============================================
	/// off screen Rendering
	///===============================================
	System::getInstance()->ScreenPreDraw();
	sceneView_->DrawTexture();
	System::getInstance()->ScreenPostDraw();
}
#pragma endregion"Game"

#pragma region"GameClear"
void GameScene::GameClearUpdate(){

	score_->UpdateOnGameClear();

	if(input_->isReleaseKey(DIK_SPACE)){
		reticle_->ResteStatus();
		beam_->ResetStatus();
		score_->Init();
		railCamera_->ResetStatus();
		enemyManager_->Init();

		currentUpdate_ = [this](){TitleUpdate(); };
		currentDraw_ = [this](){TitleDraw(); };
	}
}

void GameScene::GameClearDraw(){
	System::getInstance()->getLightManager()->Update();

	sceneView_->PreDraw();
	///===============================================
	/// 3d Object
	///===============================================
	Object3d::PreDraw();
	
	///===============================================
	/// sprite
	///===============================================
	SpriteCommon::getInstance()->PreDraw();

	titleBackground_->Draw();
	score_->Draw();

	sceneView_->PostDraw();
	///===============================================
	/// off screen Rendering
	///===============================================
	System::getInstance()->ScreenPreDraw();
	sceneView_->DrawTexture();
	System::getInstance()->ScreenPostDraw();
}
#pragma endregion"Score"
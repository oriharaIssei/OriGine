#include "GameScene.h"

#include "debugCamera/debugCamera.h"

#include <string>

#include "directX12/DxCommand.h"
#include "directX12/DxRtvArray.h"
#include "directX12/DxRtvArrayManager.h"
#include "directX12/DxSrvArrayManager.h"
#include "sprite/SpriteCommon.h"
#include "material/TextureManager.h"
#include "myFileSystem/MyFileSystem.h"
#include "primitiveDrawer/PrimitiveDrawer.h"
#include "System.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

constexpr char dockingIDName[] = "ObjectsWindow";

GameScene::~GameScene(){
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

	textureList_ = myFs::SearchFile("./resource","png");
	objectList_ = myFs::SearchFile("./resource","obj");

	emitter_.Init(10,materialManager_);
}

void GameScene::Update(){
#ifdef _DEBUG
	debugCamera_->Update();
	debugCamera_->DebugUpdate();
	cameraBuff_.openData_.viewMat = debugCamera_->getCameraTransform().viewMat;
	cameraBuff_.openData_.projectionMat = debugCamera_->getCameraTransform().projectionMat;
	cameraBuff_.ConvertToBuffer();
#endif // _DEBUG

	emitter_.Update(cameraBuff_.openData_);
}

void GameScene::Draw(){
	sceneView_->PreDraw();
	
	///===============================================
	/// 3d Object
	///===============================================
	Object3d::PreDraw();

	emitter_.Draw(cameraBuff_);

	///===============================================
	/// sprite
	///===============================================
	SpriteCommon::getInstance()->PreDraw();


	sceneView_->PostDraw();
	///===============================================
	/// off screen Rendering
	///===============================================
	System::getInstance()->ScreenPreDraw();
	sceneView_->DrawTexture();
	System::getInstance()->ScreenPostDraw();
}
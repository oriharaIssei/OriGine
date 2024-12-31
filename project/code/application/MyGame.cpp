#include "MyGame.h"

#include "Engine.h"
#include "globalVariables/GlobalVariables.h"
#include "scene/SceneManager.h"

//scene
#include "scene/AnimationEditScene.h"
#include "scene/GameScene.h"

MyGame::MyGame(){}

MyGame::~MyGame(){}

void MyGame::Init(){
	engine_ = Engine::getInstance();

	variables_ = GlobalVariables::getInstance();
	sceneManager_ = SceneManager::getInstance();

	variables_->LoadAllFile();
	engine_->Init();
	sceneManager_->Init();

    //exe 上で 使用するscene
    sceneManager_->addScene("GameScene",std::make_unique<GameScene>());
    sceneManager_->addScene("AnimationEditScene",std::make_unique<AnimationEditScene>());
    sceneManager_->changeScene("GameScene");
}

void MyGame::Finalize(){
	sceneManager_->Finalize();
	engine_->Finalize();
}

void MyGame::Run(){
	while(!engine_->ProcessMessage()){
		engine_->BeginFrame();

		variables_->Update();

		sceneManager_->Update();
		sceneManager_->Draw();

		engine_->EndFrame();
	}
}

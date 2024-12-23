#include "MyGame.h"

#include "Engine.h"
#include "globalVariables/GlobalVariables.h"
#include "scene/SceneManager.h"

#include "scene/TitleScene.h"

MyGame::MyGame(){}

MyGame::~MyGame(){}

void MyGame::Init(){
	engine_ = Engine::getInstance();

	variables_ = GlobalVariables::getInstance();
	sceneManager_ = SceneManager::getInstance();

	variables_->LoadAllFile();
	engine_->Init();
	sceneManager_->Init();
	sceneManager_->ChangeScene(std::make_unique<TitleScene>());
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

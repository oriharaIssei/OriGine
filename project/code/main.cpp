#include "directX12/DxDebug.h"
#include <Engine.h>

#include <memory>

#include "DeltaTime/DeltaTime.h"
#include "globalVariables/GlobalVariables.h"
#include <application/scene/GameScene.h>
#include <application/scene/SceneManager.h>

int WINAPI WinMain(HINSTANCE,HINSTANCE,LPSTR,int){
	DxDebug debug;
	Engine* system = Engine::getInstance();
	GlobalVariables* variables = GlobalVariables::getInstance();
	SceneManager* sceneManager = SceneManager::getInstance();

	variables->LoadAllFile();
	system->Init();
	sceneManager->Init();
	sceneManager->ChangeScene(std::make_unique<GameScene>());

	while(!system->ProcessMessage()){
		system->BeginFrame();

		variables->Update();

		sceneManager->Update();
		sceneManager->Draw();

		system->EndFrame();
	}

	sceneManager->Finalize();
	system->Finalize();

	return 0;
}
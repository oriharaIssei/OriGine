#include "directX12/DxDebug.h"
#include <System.h>

#include <memory>

#include "DeltaTime/DeltaTime.h"
#include "globalVariables/GlobalVariables.h"
#include <application/gameScene/GameScene.h>

int WINAPI WinMain(HINSTANCE,HINSTANCE,LPSTR,int){
	DxDebug debug;
	System* system = System::getInstance();
	GlobalVariables* variables = GlobalVariables::getInstance();
	std::unique_ptr<GameScene> scene = std::make_unique<GameScene>();

	variables->LoadAllFile();
	system->Init();
	scene->Init();

	while(!system->ProcessMessage()){
		system->BeginFrame();

		variables->Update();

		scene->Update();
		scene->Draw();

		system->EndFrame();
	}

	system->Finalize();
	return 0;
}
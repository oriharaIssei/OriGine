#include "directX12/dxDebug/DxDebug.h"
#include <System.h>

#include <memory>

#include "DeltaTime/DeltaTime.h"
#include "globalVariables/GlobalVariables.h"
#include <GameScene.h>

int WINAPI WinMain(HINSTANCE,HINSTANCE,LPSTR,int){
	DxDebug debug;
	System* system = System::getInstance();
	DeltaTime* deltaTime = DeltaTime::getInstance();
	GlobalVariables* variables = GlobalVariables::getInstance();
	std::unique_ptr<GameScene> scene = std::make_unique<GameScene>();

	system->Init();
	variables->LoadAllFile();
	scene->Init();
	deltaTime->Init();

	while(!system->ProcessMessage()){
		deltaTime->Update();
		system->BeginFrame();

		variables->Update();

		scene->Update();
		scene->Draw();

		system->EndFrame();
	}

	system->Finalize();
	return 0;
}
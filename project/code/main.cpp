#include <memory>

#include "application/MyGame.h"
#include "directX12/DxDebug.h"

int WINAPI WinMain(HINSTANCE,HINSTANCE,LPSTR,int){
	DxDebug debug;

	std::unique_ptr<MyGame> gameApp = std::make_unique<MyGame>();
	gameApp->Init();

	gameApp->Run();

	gameApp->Finalize();

	return 0;
}
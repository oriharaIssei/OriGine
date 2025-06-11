#include "engine/code/directX12/DxDebug.h"

#include "application/code/MyGame.h"
#include "lib/logger/Logger.h"

#include <memory>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    DxDebug::getInstance()->InitializeDebugger();

    Logger::Initialize();
    {
        std::unique_ptr<MyGame> gameApp = std::make_unique<MyGame>();

        gameApp->Initialize();
        DxDebug::getInstance()->CreateInfoQueue();

        gameApp->Run();

        gameApp->Finalize();
    }
    Logger::Finalize();

    return 0;
}

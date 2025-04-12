#include "engine/code/directX12/DxDebug.h"

#include "application/code/MyGame.h"
#include "lib/logger/Logger.h"

#include <memory>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Logger::Initialize();
    {
        DxDebug debug;
        std::unique_ptr<MyGame> gameApp = std::make_unique<MyGame>();

        gameApp->Initialize();

        gameApp->Run();

        gameApp->Finalize();
    }
    return 0;
}

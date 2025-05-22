#include "engine/code/directX12/DxDebug.h"

#include "application/code/MyGame.h"
#include "lib/logger/Logger.h"

#include <memory>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Logger::Initialize();
    {
        DxDebug debug;
        Logger::setDxDebug(&debug);
        std::unique_ptr<MyGame> gameApp = std::make_unique<MyGame>();

        gameApp->Initialize();
        debug.CreateInfoQueue();

        gameApp->Run();

        gameApp->Finalize();

        Logger::setDxDebug(nullptr);
    }
    Logger::Finalize();

    return 0;
}

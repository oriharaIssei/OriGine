#include "engine/code/directX12/DxDebug.h"

#include "application/code/MyGame.h"

#include <memory>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    {
        DxDebug debug;
        std::unique_ptr<MyGame> gameApp = std::make_unique<MyGame>();

        gameApp->Initialize();

        gameApp->Run();

        gameApp->Finalize();
    }
    return 0;
}

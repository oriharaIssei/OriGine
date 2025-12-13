#include "SceneManager.h"

// Interface
#include "scene/Scene.h"

/// stl
#include <format>

/// engine
#include "Engine.h"
#include "logger/Logger.h"
#include "winApp/WinApp.h"
// input
#include "input/GamepadInput.h"
#include "input/InputManager.h"
#include "input/KeyboardInput.h"
#include "input/MouseInput.h"

#define ENGINE_INPUT
#define RESOURCE_DIRECTORY
#define DELTA_TIME
#include "EngineInclude.h"
// ecs
#include "component/ComponentArray.h"
#include "entity/Entity.h"
#include "system/ISystem.h"
#include "system/SystemRunner.h"

// directX12Object
#include "directX12/RenderTexture.h"
// module
#include "camera/CameraManager.h"
#include "editor/EditorController.h"
#include "texture/TextureManager.h"
// util
#include "myFileSystem/MyFileSystem.h"

/// math
#include "math/Vector2.h"
#include "math/Vector4.h"

using namespace OriGine;

#pragma region "SceneManager"

SceneManager::SceneManager() {}
SceneManager::~SceneManager() {}

void SceneManager::Initialize(const ::std::string& _startScene, KeyboardInput* _keyInput, MouseInput* _mouseInput, GamepadInput* _padInput) {
    // 入力デバイスの設定
    keyInput_   = _keyInput;
    mouseInput_ = _mouseInput;
    padInput_   = _padInput;

    // シーンの初期化
    currentScene_ = ::std::make_unique<Scene>(_startScene);
    // シーンの入力デバイスを設定
    currentScene_->SetInputDevices(keyInput_, mouseInput_, padInput_);
    // シーンの初期化処理
    currentScene_->Initialize();
    // シーンビューの初期化
    currentScene_->GetSceneView()->Resize(Engine::GetInstance()->GetWinApp()->GetWindowSize());
    // シーンマネージャーの設定 (this)
    currentScene_->SetSceneManager(this);

#ifdef _DEVELOP
    fileWatcher_ = ::std::make_unique<FileWatcher>(kApplicationResourceDirectory + "/scene/" + _startScene + ".json");
    fileWatcher_->Start();
#endif // _DEVELOP
}
void SceneManager::Initialize(KeyboardInput* _keyInput, MouseInput* _mouseInput, GamepadInput* _padInput) {
    this->Initialize(startupSceneName_, _keyInput, _mouseInput, _padInput);
}

void SceneManager::Finalize() {
#ifdef _DEVELOP
    fileWatcher_->Stop();
#endif // _DEVELOP
}

void SceneManager::Update() {
#ifdef _DEVELOP
    if (fileWatcher_->isChanged()) {
        this->ChangeScene(currentScene_->GetName());
    }
#endif // _DEVELOP

    currentScene_->Update();
}

void SceneManager::Render() {
    currentScene_->Render();
}

const ::std::string& SceneManager::GetCurrentSceneName() const { return currentScene_->GetName(); }

void SceneManager::ChangeScene(const ::std::string& name) {
    changingSceneName_ = name;
    isChangeScene_     = true;
}

void SceneManager::ExecuteSceneChange() {
    LOG_TRACE("SceneChange\n PreviousScene : [ {} ] \n NextScene : [ {} ]", currentScene_->GetName(), changingSceneName_);

    currentScene_->Finalize();
    currentScene_.reset();
    currentScene_ = ::std::make_unique<Scene>(changingSceneName_);

    // 入力デバイスの設定
    currentScene_->SetInputDevices(keyInput_, mouseInput_, padInput_);
    // シーンの初期化処理
    currentScene_->Initialize();
    // シーンビューの初期化
    currentScene_->GetSceneView()->Resize(Engine::GetInstance()->GetWinApp()->GetWindowSize());
    // シーンマネージャーの設定 (this)
    currentScene_->SetSceneManager(this);

#ifdef _DEVELOP
    // 監視対象を変更
    fileWatcher_->Stop();
    fileWatcher_->SetFilePath(kApplicationResourceDirectory + "/scene/" + changingSceneName_ + ".json");
    fileWatcher_->Start();
#endif // _DEVELOP

    isChangeScene_ = false;
}
#pragma endregion

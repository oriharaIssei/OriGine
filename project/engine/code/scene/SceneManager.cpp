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

/// <summary>
/// シーンマネージャーの初期化. 最初のシーンを生成し、入力デバイスの紐付けを行う.
/// </summary>
/// <param name="_startScene">最初にロードするシーン名</param>
/// <param name="_keyInput">キーボード入力へのポインタ</param>
/// <param name="_mouseInput">マウス入力へのポインタ</param>
/// <param name="_padInput">ゲームパッド入力へのポインタ</param>
void SceneManager::Initialize(const ::std::string& _startScene, KeyboardInput* _keyInput, MouseInput* _mouseInput, GamepadInput* _padInput) {
    // 入力デバイスの設定。各シーンに引き継がれる
    keyInput_   = _keyInput;
    mouseInput_ = _mouseInput;
    padInput_   = _padInput;

    // シーンの初期化。指定された名前で Scene インスタンスを生成
    currentScene_ = ::std::make_unique<Scene>(_startScene);
    // シーンに入力デバイスをセット
    currentScene_->SetInputDevices(keyInput_, mouseInput_, padInput_);
    // シーンの内部初期化 (ECS、システム、外部データのロード)
    currentScene_->Initialize();
    // 描画先バッファを現在のウィンドウサイズに合わせる
    currentScene_->GetSceneView()->Resize(Engine::GetInstance()->GetWinApp()->GetWindowSize());
    // シーン側にマネージャーへの参照を渡す
    currentScene_->SetSceneManager(this);

#ifdef _DEVELOP
    // 開発環境ではシーンファイルの変更を監視し、自動リロードを可能にする
    fileWatcher_ = ::std::make_unique<FileWatcher>(kApplicationResourceDirectory + "/scene/" + _startScene + ".json");
    fileWatcher_->Start();
#endif // _DEVELOP
}

/// <summary>
/// デフォルトのスタートアップシーンを使用して初期化する.
/// </summary>
void SceneManager::Initialize(KeyboardInput* _keyInput, MouseInput* _mouseInput, GamepadInput* _padInput) {
    this->Initialize(startupSceneName_, _keyInput, _mouseInput, _padInput);
}

/// <summary>
/// 終了処理.
/// </summary>
void SceneManager::Finalize() {
#ifdef _DEVELOP
    fileWatcher_->Stop();
#endif // _DEVELOP
}

/// <summary>
/// 現在のシーンの更新処理.
/// 開発時はファイルの変更をチェックし、変更があればシーンをリロードする.
/// </summary>
void SceneManager::Update() {
#ifdef _DEVELOP
    if (fileWatcher_->isChanged()) {
        // ファイルが更新されたら即座にそのシーンを再起動 (ホットリロード)
        this->ChangeScene(currentScene_->GetName());
    }
#endif // _DEVELOP

    currentScene_->Update();
}

/// <summary>
/// 現在のシーンの描画コマンド実行.
/// </summary>
void SceneManager::Render() {
    currentScene_->Render();
}

/// <summary>
/// 現在アクティブなシーンの名前を取得する.
/// </summary>
const ::std::string& SceneManager::GetCurrentSceneName() const { return currentScene_->GetName(); }

/// <summary>
/// 次のフレームで指定されたシーンに切り替えるように予約する.
/// </summary>
/// <param name="_name">遷移先のシーン名</param>
void SceneManager::ChangeScene(const ::std::string& _name) {
    changingSceneName_ = _name;
    isChangeScene_     = true;
}

/// <summary>
/// 予約されていたシーン切り替えを実際に実行する.
/// 現在のシーンを終了 (Finalize) し、新しいシーンを構築・初期化する.
/// </summary>
void SceneManager::ExecuteSceneChange() {
    LOG_TRACE("SceneChange\n PreviousScene : [ {} ] \n NextScene : [ {} ]", currentScene_->GetName(), changingSceneName_);

    // 現行シーンの解放
    currentScene_->Finalize();
    currentScene_.reset();

    // 新シーンの生成
    currentScene_ = ::std::make_unique<Scene>(changingSceneName_);

    // 入力デバイスの継承設定
    currentScene_->SetInputDevices(keyInput_, mouseInput_, padInput_);
    // 新シーンの初期構築
    currentScene_->Initialize();
    // 新シーンのレンダーターゲットを現在の画面解像度に最適化
    currentScene_->GetSceneView()->Resize(Engine::GetInstance()->GetWinApp()->GetWindowSize());
    // シーンマネージャーへの逆参照をセット
    currentScene_->SetSceneManager(this);

#ifdef _DEVELOP
    // 開発時は監視対象のファイルを新シーンのものに差し替える
    fileWatcher_->Stop();
    fileWatcher_->SetFilePath(kApplicationResourceDirectory + "/scene/" + changingSceneName_ + ".json");
    fileWatcher_->Start();
#endif // _DEVELOP

    // 切り替えフラグを下ろす
    isChangeScene_ = false;
}
#pragma endregion

#pragma once

/// stl
#include <memory>
// string
#include <string>

/// ECS
// entity
#include "entity/Entity.h"
// component
#include "component/ComponentArray.h"

/// util
#include "util/globalVariables/SerializedField.h"
#include "util/myFileSystem/MyFileSystem.h"

/// externals
#include <binaryIO/BinaryIO.h>
#include <nlohmann/json.hpp>

namespace OriGine {

/// engine
class Scene;
// input
class KeyboardInput;
class MouseInput;
class GamepadInput;

/// <summary>
/// アプリケーション全体のシーン遷移や、現在のシーンのライフサイクルを管理するクラス.
/// シーンの切り替え、ゲーム終了フラグの管理、開発環境下でのアセット変更監視などを担当する.
/// </summary>
class SceneManager {
    friend class ReplayPlayer;

public:
    SceneManager();
    ~SceneManager();

    /// <summary>
    /// シーンマネージャーを初期化し、最初のシーンを開始する.
    /// </summary>
    /// <param name="_startScene">開始するシーンの名前</param>
    /// <param name="_keyInput">キーボード入力</param>
    /// <param name="_mouseInput">マウス入力</param>
    /// <param name="_padInput">ゲームパッド入力</param>
    void Initialize(const ::std::string& _startScene, KeyboardInput* _keyInput, MouseInput* _mouseInput, GamepadInput* _padInput);

    /// <summary>
    /// シーンマネージャーを初期化する (開始シーン指定なし版).
    /// </summary>
    void Initialize(KeyboardInput* _keyInput, MouseInput* _mouseInput, GamepadInput* _padInput);

    /// <summary>
    /// 保持している現在のシーンを終了させ、リソースを解放する.
    /// </summary>
    void Finalize();

    /// <summary>
    /// 現在のシーンの更新処理を呼び出す. シーン変更が予約されている場合は切り替え処理も行う.
    /// </summary>
    void Update();

    /// <summary>
    /// 現在のシーンの描画処理を呼び出す.
    /// </summary>
    void Render();

    /// <summary>
    /// 予約されているシーン変更を即座に実行する.
    /// 通常はフレーム間の適切なタイミングで自動的に呼び出される.
    /// </summary>
    void ExecuteSceneChange();

private:
#ifdef _DEVELOP
    /// <summary>エディタ上でのアセット変更を監視するファイルウォッチャー</summary>
    ::std::unique_ptr<FileWatcher> fileWatcher_ = nullptr;
#endif

    /// <summary>起動時に読み込むシーン名 (外部設定ファイル等と同期するフィールド)</summary>
    SerializedField<::std::string> startupSceneName_ = SerializedField<::std::string>("Settings", "Scene", "StartupSceneName");

    /// <summary>現在アクティブなシーン</summary>
    ::std::unique_ptr<Scene> currentScene_ = nullptr;

    // --- Input Devices ---
    KeyboardInput* keyInput_ = nullptr;
    MouseInput* mouseInput_  = nullptr;
    GamepadInput* padInput_  = nullptr;

    /// <summary>次に切り替える予定のシーン名</summary>
    ::std::string changingSceneName_ = "";

    /// <summary>シーン変更がリクエストされているか</summary>
    bool isChangeScene_ = false;
    /// <summary>現在のフレーム内にシーンを変更すべきか</summary>
    bool changeSceneInFrame_ = false;
    /// <summary>アプリケーションを終了すべきかどうかのフラグ</summary>
    bool isExitGame_ = false;

public:
    /// <summary>現在アクティブなシーンへのポインタを取得する.</summary>
    Scene* GetCurrentScene() {
        return currentScene_.get();
    }

    /// <summary>設定されているスタートアップシーン名のフィールドを取得する.</summary>
    const SerializedField<::std::string>& GetStartupSceneName() const {
        return startupSceneName_;
    }

    /// <summary>スタートアップシーン名のフィールドをリファレンスで取得する (編集用).</summary>
    SerializedField<::std::string>& GetStartupSceneNameRef() {
        return startupSceneName_;
    }

    /// <summary>ゲーム終了リクエストが発生しているか取得する.</summary>
    bool IsExitGame() const {
        return isExitGame_;
    }

    /// <summary>ゲームの終了フラグを設定する (true を渡すとアプリケーションが終了へ向かう).</summary>
    void SetExitGame(bool exit) { isExitGame_ = exit; }

    /// <summary>現在実行中のシーン名を取得する.</summary>
    const ::std::string& GetCurrentSceneName() const;

    /// <summary>
    /// 次のフレームでのシーン切り替えを予約する.
    /// </summary>
    /// <param name="name">切り替え先のシーン名</param>
    void ChangeScene(const ::std::string& name);

    /// <summary>現在シーンの切り替え中 (予約中) かどうかを取得する.</summary>
    bool IsChangeScene() const { return isChangeScene_; }
};

} // namespace OriGine

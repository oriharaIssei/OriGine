#include "SubScene.h"

#ifdef _DEBUG
/// engine
#define RESOURCE_DIRECTORY
#include <EngineInclude.h>

/// utils
#include "myFileSystem/MyFileSystem.h"
#include "myGui/MyGui.h"
#endif // _DEBUG

using namespace OriGine;

SubScene::SubScene() {}
SubScene::~SubScene() {}

void SubScene:: Initialize(Scene* /*_scene*/, const EntityHandle& /*_owner*/) {
    // アクティブ状態でシーン名が設定済みの場合は初期化時点で読み込む
    if (!sceneName_.empty() && isActive_) {
        Load(sceneName_);
    }
}

void SubScene::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] const EntityHandle& _handle, [[maybe_unused]] const ::std::string& _parentLabel) {
#ifdef _DEBUG

    CheckBoxCommand("IsActive##" + _parentLabel, isActive_);

    InputGuiCommand("RenderingPriority##" + _parentLabel, renderingPriority_, "%d");

    ImGui::Spacing();

    ::std::string label = "SceneName##" + _parentLabel;
    if (ImGui::BeginCombo(label.c_str(), sceneName_.c_str())) {
        ::std::list<::std::pair<::std::string, ::std::string>> sceneList = myfs::SearchFile(kApplicationResourceDirectory + "/scene", "json");
        for (const auto& scene : sceneList) {
            bool isSelected = (sceneName_ == scene.second);
            if (ImGui::Selectable(scene.second.c_str(), isSelected)) {
                // シーン名変更時のコールバックで、旧シーンを解放してから新シーンを読み込み直す
                auto command = ::std::make_unique<SetterCommand<::std::string>>(&sceneName_, scene.second, [this](::std::string* _newScene) {
                    if (this) {
                        Unload();
                        Load(*_newScene);
                    }
                });
                OriGine::EditorController::GetInstance()->PushCommand(::std::move(command));
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
#endif // _DEBUG
}

/// <summary>
/// 保持しているサブシーンを解放する
/// </summary>
void SubScene::Finalize() {
    Unload();
}

void SubScene::Activate() {
    isActive_ = true;
    // 既にロード済みのサブシーンがあれば使い回し、無い場合のみ新規にロードする
    if (!subScene_) {
        Load(sceneName_);
    }
}

void SubScene::Deactivate() {
    isActive_ = false;
    // 非アクティブ化と同時にサブシーンを解放する
    // (SceneManager の管理下ではないため、ここで明示的に解放しないとメモリ上に残り続ける)
    Unload();
}

void SubScene::Load(const ::std::string& _sceneName) {
    sceneName_ = _sceneName;
    // このコンポーネントが Scene インスタンスを所有する(SceneManager には登録されない)
    subScene_  = ::std::make_unique<Scene>(sceneName_);
    subScene_->Initialize();
}
void SubScene::Unload() {
    if (subScene_) {
        subScene_->Finalize();
        subScene_.reset();
    }
}

/// <summary>
/// SubScene を JSON へ書き出す
/// </summary>
void OriGine::to_json(nlohmann::json& j, const SubScene& scene) {
    j = nlohmann::json{
        {"isActive", scene.isActive_},
        {"sceneName", scene.sceneName_},
        {"renderingPriority", scene.renderingPriority_},
    };
}
/// <summary>
/// JSON から SubScene を復元する
/// </summary>
void OriGine::from_json(const nlohmann::json& j, SubScene& scene) {
    j.at("isActive").get_to(scene.isActive_);
    j.at("sceneName").get_to(scene.sceneName_);
    // 後方互換: 古いセーブデータにrenderingPriorityが無い場合はデフォルト値のままにする
    if (j.contains("renderingPriority")) {
        j.at("renderingPriority").get_to(scene.renderingPriority_);
    }
}

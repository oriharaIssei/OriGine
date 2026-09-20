#include "EntityReferenceList.h"

/// engine
#define RESOURCE_DIRECTORY
#include "EngineInclude.h"

/// editor
#include "editor/EditorController.h"
#include "myGui/MyGui.h"
/// util
#include "myFileSystem/MyFileSystem.h"

using namespace OriGine;

/// <summary>
/// コンストラクタ
/// </summary>
EntityReferenceList::EntityReferenceList() {}

/// <summary>
/// デストラクタ
/// </summary>
EntityReferenceList::~EntityReferenceList() {}

/// <summary>
/// 初期化処理。
/// 参照リストの中身はシーン読み込み時に from_json 経由で復元されるため、ここでは何もしない
/// </summary>
void EntityReferenceList:: Initialize(Scene* /*_scene*/, const EntityHandle& /*_owner*/) {}

/// <summary>
/// デバッグ用GUIで、参照するエンティティファイルの一覧を編集する
/// </summary>
/// <param name="_parentLabel">ImGuiのID衝突を避けるための親ラベル</param>
void EntityReferenceList::Edit(Scene* /*_scene*/, const EntityHandle& /*_owner*/, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef ORIGINE_EDITOR_ENABLED

    std::string label = "##" + _parentLabel;

    // 参照済みエンティティファイルの一覧表示 + 個別削除ボタン
    for (size_t i = 0; i < entityFileList_.size(); ++i) {
        ImGui::PushID(static_cast<int>(i));
        std::string filePath = entityFileList_[i].first + "/" + entityFileList_[i].second;
        ImGui::Text("%s", filePath.c_str());

        ImGui::SameLine();

        label = "Remove##" + filePath + _parentLabel;
        if (ImGui::Button(label.c_str())) {
            // 直接eraseせず、EditorController内のキューに積むCommandパターン経由にすることで、
            // エディタのUndo/Redoに対応しつつ実際のerase実行を後のタイミング(コマンド処理時)に遅延させる
            auto command = std::make_unique<EraseElementCommand<std::vector<std::pair<std::string, std::string>>>>(&entityFileList_, entityFileList_.begin() + i);
            OriGine::EditorController::GetInstance()->PushCommand(std::move(command));

            // entityFileList_.begin() + i を保持したコマンドを積んだ直後にループを続けると、
            // 後続要素へのイテレータ/インデックスの前提が崩れる可能性があるため、同じフレームでは抜ける
            ImGui::PopID();
            break;
        }

        ImGui::PopID();
    }

    // ファイルダイアログで新規エンティティファイルを選択し、参照リストへ追加
    label = "Add Entity Reference##" + _parentLabel;
    if (ImGui::Button(label.c_str())) {
        std::string directory, filename;
        if (MyFileSystem::SelectFileDialog(kApplicationResourceDirectory, directory, filename, {"ent"}, true)) {
            auto command = std::make_unique<AddElementCommand<std::vector<std::pair<std::string, std::string>>>>(&entityFileList_, std::make_pair(kApplicationResourceDirectory + "/" + directory, filename));
            OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
        }
    }

#endif // _DEBUG
}

/// <summary>
/// 終了処理。ファイルパスの文字列しか保持しておらず、解放すべきリソースが無いため何もしない
/// </summary>
void EntityReferenceList::Finalize() {}

/// <summary>
/// entityFileList_をjsonへ書き出す
/// </summary>
/// <param name="j">書き込み先のJSON</param>
/// <param name="c">シリアライズ対象のEntityReferenceList</param>
void OriGine::to_json(nlohmann::json& j, const EntityReferenceList& c) {
    j = nlohmann::json{{"entityFileList", c.entityFileList_}};
}

/// <summary>
/// jsonからentityFileList_を復元する
/// </summary>
/// <param name="j">読み込み元のJSON</param>
/// <param name="c">復元先のEntityReferenceList</param>
void OriGine::from_json(const nlohmann::json& j, EntityReferenceList& c) {
    j.at("entityFileList").get_to(c.entityFileList_);
}

#include "CollisionPushBackInfo.h"

#ifdef _DEBUG
#include "myGui/MyGui.h"
#endif // _DEBUG

using namespace OriGine;

/// <summary>
/// 終了処理。蓄積された衝突情報をすべて破棄する
/// </summary>
void CollisionPushBackInfo::Finalize() {
    collisionInfoMap_.clear();
}

/// <summary>
/// デバッグ用GUIで押し戻しの種類（PushBack / Reflect / None）を選択する
/// </summary>
/// <param name="_parentLabel">ImGuiのID衝突を避けるための親ラベル</param>
void CollisionPushBackInfo::Edit(Scene* /*_scene*/, const EntityHandle& /*_entity*/, [[maybe_unused]] [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG
    // エディタ専用のUIコードなので、リリースビルドには含めない

    // 選択結果を直接代入せずSetterCommand経由にすることで、エディタのUndo/Redoに対応させる
    std::string label = "PushBackType##" + _parentLabel;
    if (ImGui::BeginCombo(label.c_str(), GetCollisionPushBackTypeName(pushBackType_))) {
        for (int i = 0; i < static_cast<int>(CollisionPushBackType::Count); ++i) {
            CollisionPushBackType type = static_cast<CollisionPushBackType>(i);
            bool isSelected            = (pushBackType_ == type);
            if (ImGui::Selectable(GetCollisionPushBackTypeName(type), isSelected)) {
                auto command = std::make_unique<SetterCommand<CollisionPushBackType>>(&pushBackType_, type);
                OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
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
/// 前フレームに蓄積した衝突情報を破棄する。
/// 押し戻し情報は1フレーム限りの使い捨てなので、衝突判定を始める前に毎フレーム呼んで
/// 前フレーム分が残ったまま二重に押し戻されるのを防ぐ
/// </summary>
void CollisionPushBackInfo::ClearInfo() {
    collisionInfoMap_.clear();
}

/// <summary>
/// 押し戻し種別を表示用の文字列に変換する
/// </summary>
/// <param name="_type">変換する押し戻し種別</param>
/// <returns>種別に対応する名前。未知の値やNoneの場合は "None"</returns>
const char* OriGine::GetCollisionPushBackTypeName(CollisionPushBackType _type) {
    switch (_type) {
    case CollisionPushBackType::PushBack:
        return "PushBack";
    case CollisionPushBackType::Reflect:
        return "Reflect";
    }
    return "None";
}

/// <summary>
/// CollisionPushBackInfoの状態をJSONへ書き出す。
/// collisionInfoMap_ は毎フレーム作り直される実行時の一時データなので保存対象外
/// </summary>
/// <param name="_j">書き込み先のJSON</param>
/// <param name="_comp">シリアライズ対象のCollisionPushBackInfo</param>
void OriGine::to_json(nlohmann::json& _j, const CollisionPushBackInfo& _comp) {
    _j = nlohmann::json{
        {"pushBackType", _comp.pushBackType_}};
}

/// <summary>
/// JSONからCollisionPushBackInfoの状態を復元する
/// </summary>
/// <param name="_j">読み込み元のJSON</param>
/// <param name="_comp">復元先のCollisionPushBackInfo</param>
void OriGine::from_json(const nlohmann::json& _j, CollisionPushBackInfo& _comp) {
    _j.at("pushBackType").get_to(_comp.pushBackType_);
};

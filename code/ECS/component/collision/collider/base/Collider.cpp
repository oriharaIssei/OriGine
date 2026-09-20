#include "Collider.h"

#include "CollisionCategoryManager.h"

#ifdef ORIGINE_EDITOR_ENABLED
#include "myGui/MyGui.h"
#endif // _DEBUG

using namespace OriGine;

/// <summary>
/// デバッグ用GUIで有効/無効フラグと衝突カテゴリを編集する（各派生コライダーのEditから呼ばれる共通部分）
/// </summary>
/// <param name="_parentLabel">ImGuiのID衝突を避けるための親ラベル</param>
void OriGine::ICollider::Edit(Scene* /*_scene*/, const EntityHandle& /*_handle*/, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef ORIGINE_EDITOR_ENABLED
    // エディタ専用のUIコードなので、リリースビルドには含めない

    CheckBoxCommand("IsActive##" + _parentLabel, isActive_);

    // カテゴリ選択Combo
    auto* manager                          = CollisionCategoryManager::GetInstance();
    const auto& categories                 = manager->GetCategories();
    const std::string& currentCategoryName = collisionCategory_.GetName();
    std::string comboLabel                 = "Category##" + _parentLabel;

    if (ImGui::BeginCombo(comboLabel.c_str(), currentCategoryName.c_str())) {
        for (const auto& [name, category] : categories) {
            bool isSelected = (currentCategoryName == name);
            if (ImGui::Selectable(name.c_str(), isSelected)) {
                collisionCategory_ = category;
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

#endif // _DEBUG
}

void ICollider::StartCollision() {
    // 前フレームの状態を退避してから今フレーム分をクリアする(形状によらない共通部分のみ)。
    // ワールド形状の再計算(旧: ここでCalculateWorldShape()を仮想呼び出ししていた)は、
    // 形状型を知らないICollider側では行えなくなったため、Collider<BoundsClass>::StartCollision()
    // がこの関数を呼んだ直後に自分でCalculateWorldShape()を呼ぶ形に変わった(Phase 3 3B)。
    this->preCollisionStateMap_ = this->collisionStateMap_;
    this->collisionStateMap_.clear();
}

void ICollider::EndCollision() {
    // 前フレーム衝突していた相手のうち、今フレームで更新されなかったものをExit状態にする
    for (auto& [entity, state] : this->preCollisionStateMap_) {
        // 前フレームで既にExitを通知済みの相手は、Exitを二重に発火させないようスキップする。
        // ここでreturnしてしまうと、残りの相手のExit判定まで打ち切られてしまう
        if (state == CollisionState::Exit)
            continue;
        if (this->collisionStateMap_[entity] == CollisionState::None)
            this->collisionStateMap_[entity] = CollisionState::Exit;
    }
}

/// <summary>
/// ICollider共通部分（有効フラグ・衝突カテゴリ名）をJSONへ書き出す
/// </summary>
/// <param name="_j">書き込み先のJSON</param>
/// <param name="_c">シリアライズ対象のICollider</param>
void OriGine::to_json(nlohmann::json& _j, const ICollider& _c) {
    _j["isActive"]          = _c.isActive_;
    _j["collisionCategory"] = _c.collisionCategory_.GetName();
}

/// <summary>
/// JSONからICollider共通部分（有効フラグ・衝突カテゴリ）を復元する
/// </summary>
/// <param name="_j">読み込み元のJSON</param>
/// <param name="_c">復元先のICollider</param>
void OriGine::from_json(const nlohmann::json& _j, ICollider& _c) {
    // 古いセーブデータにキーが無い場合でも読み込みが壊れないよう、
    // 各項目の存在をcontains()で確認してから取得する
    if (_j.contains("isActive")) {
        _c.isActive_ = _j["isActive"].get<bool>();
    }

    if (_j.contains("collisionCategory")) {
        std::string categoryName          = _j["collisionCategory"].get<std::string>();
        CollisionCategoryManager* manager = CollisionCategoryManager::GetInstance();
        _c.collisionCategory_             = manager->GetCategory(categoryName);
    }
}

/// <summary>
/// 相手のコライダーと衝突可能か判定する
/// </summary>
/// <param name="_other">相手のコライダー</param>
/// <returns>双方のカテゴリマスクが互いを許可していればtrue</returns>
bool ICollider::CanCollideWith(const ICollider& _other) const {
    uint32_t maskA = collisionCategory_.GetMaskBits();
    uint32_t maskB = _other.collisionCategory_.GetMaskBits();
    uint32_t bitsA = collisionCategory_.GetBits();
    uint32_t bitsB = _other.collisionCategory_.GetBits();

    // 双方向でマスクをチェック（AがBを許可 かつ BがAを許可）
    bool aAllowsB = (maskA & bitsB) != 0;
    bool bAllowsA = (maskB & bitsA) != 0;
    return aAllowsB && bAllowsA;
}

#include "AABBCollider.h"

namespace OriGine {
/// <summary>
/// AABBColliderの状態をJSONへ書き出す
/// </summary>
/// <param name="_json">書き込み先のJSON</param>
/// <param name="_a">シリアライズ対象のAABBCollider</param>
void to_json(nlohmann::json& _json, const AABBCollider& _a) {
    to_json(_json, static_cast<const ICollider&>(_a));
    _json["center"]    = _a.shape_.center;
    _json["halfSize"]  = _a.shape_.halfSize;
    _json["transform"] = _a.transform_;
}
/// <summary>
/// JSONからAABBColliderの状態を復元する
/// </summary>
/// <param name="_json">読み込み元のJSON</param>
/// <param name="_a">復元先のAABBCollider</param>
void from_json(const nlohmann::json& _json, AABBCollider& _a) {
    from_json(_json, static_cast<ICollider&>(_a));
    // 古いセーブデータにキーが無い場合でも読み込みが壊れないよう、
    // 各項目の存在をcontains()で確認してから取得する
    if (_json.contains("center")) {
        _json.at("center").get_to(_a.shape_.center);
    }
    if (_json.contains("halfSize")) {
        _json.at("halfSize").get_to(_a.shape_.halfSize);
    }
    if (_json.contains("transform")) {
        _json.at("transform").get_to(_a.transform_);
    }
}

/// <summary>
/// デバッグ用GUIでAABB形状とTransformのパラメータを編集する
/// </summary>
/// <param name="_scene">対象シーン</param>
/// <param name="_handle">対象エンティティ</param>
/// <param name="_parentLabel">ImGuiのID衝突を避けるための親ラベル</param>
void AABBCollider::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] const EntityHandle& _handle, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG
    // エディタ専用のUIコードなので、リリースビルドには含めない

    ICollider::Edit(_scene, _handle, _parentLabel);

    std::string label = "AABB##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        DragGuiVectorCommand<3, float>("Center##" + _parentLabel, this->shape_.center, 0.01f);
        DragGuiVectorCommand<3, float>("HalfSize##" + _parentLabel, this->shape_.halfSize, 0.01f);
        ImGui::TreePop();
    }
    label = "Transform##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        transform_.Edit(_scene, _handle, _parentLabel);
        ImGui::TreePop();
    }

#endif // _DEBUG
}

/// <summary>
/// ローカル形状(_local)とTransformの現在値から、ワールド空間のAABB(_world)を再計算する。
/// Collider&lt;Bounds::AABB&gt;::CalculateWorldShape()から形状型で選ばれて呼ばれる(Phase 3 3B)。
/// </summary>
void Bounds::CalculateWorldShape(const Bounds::AABB& _local, Bounds::AABB& _world, const Transform& _transform) {
    // 中心は平行移動を含むワールド行列全体で変換する。
    // ハーフサイズは向きを持たない「大きさ」なので回転は反映せず、ワールドスケールのみを軸ごとに乗算する
    // （回転まで反映すると軸並行を維持できなくなり、AABBとしての前提が崩れる）
    _world.center   = _local.center * _transform.worldMat;
    _world.halfSize = _local.halfSize * _transform.GetWorldScale();
}

/// <summary>
/// ワールド空間のAABBを取得する
/// </summary>
/// <returns>広域フェーズ（空間ハッシュ登録など）に使うワールド空間の外接AABB</returns>
Bounds::AABB AABBCollider::ToWorldAABB() const {
    return worldShape_;
}

} // namespace OriGine

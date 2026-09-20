#include "CapsuleCollider.h"

namespace OriGine {

/// <summary>
/// CapsuleColliderの状態をJSONへ書き出す
/// </summary>
/// <param name="_json">書き込み先のJSON</param>
/// <param name="_c">シリアライズ対象のCapsuleCollider</param>
void to_json(nlohmann::json& _json, const CapsuleCollider& _c) {
    to_json(_json, static_cast<const ICollider&>(_c));
    _json["start"]     = _c.shape_.segment.start;
    _json["end"]       = _c.shape_.segment.end;
    _json["radius"]    = _c.shape_.radius;
    _json["transform"] = _c.transform_;
}

/// <summary>
/// JSONからCapsuleColliderの状態を復元する
/// </summary>
/// <param name="_json">読み込み元のJSON</param>
/// <param name="_c">復元先のCapsuleCollider</param>
void from_json(const nlohmann::json& _json, CapsuleCollider& _c) {
    from_json(_json, static_cast<ICollider&>(_c));
    // 古いセーブデータにキーが無い場合でも読み込みが壊れないよう、
    // 各項目の存在をcontains()で確認してから取得する
    if (_json.contains("start")) {
        _json.at("start").get_to(_c.shape_.segment.start);
    }
    if (_json.contains("end")) {
        _json.at("end").get_to(_c.shape_.segment.end);
    }
    if (_json.contains("radius")) {
        _json.at("radius").get_to(_c.shape_.radius);
    }
    if (_json.contains("transform")) {
        _json.at("transform").get_to(_c.transform_);
    }
}

/// <summary>
/// デバッグ用GUIでCapsule形状とTransformのパラメータを編集する
/// </summary>
/// <param name="_scene">対象シーン</param>
/// <param name="_handle">対象エンティティ</param>
/// <param name="_parentLabel">ImGuiのID衝突を避けるための親ラベル</param>
void CapsuleCollider::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] const EntityHandle& _handle, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef ORIGINE_EDITOR_ENABLED
    // エディタ専用のUIコードなので、リリースビルドには含めない

    ICollider::Edit(_scene, _handle, _parentLabel);

    std::string label = "Capsule##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        DragGuiVectorCommand<3, float>("Start##" + _parentLabel, this->shape_.segment.start, 0.01f);
        DragGuiVectorCommand<3, float>("End##" + _parentLabel, this->shape_.segment.end, 0.01f);
        DragGuiCommand<float>("Radius##" + _parentLabel, this->shape_.radius, 0.01f);
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
/// ローカル形状(_local)とTransformの現在値から、ワールド空間のCapsule(_world)を再計算する。
/// Collider&lt;Bounds::Capsule&gt;::CalculateWorldShape()から形状型で選ばれて呼ばれる(Phase 3 3B)。
/// </summary>
void Bounds::CalculateWorldShape(const Bounds::Capsule& _local, Bounds::Capsule& _world, const Transform& _transform) {
    // start/endは位置なので、平行移動を含むワールド行列全体で変換する
    _world.segment.start = _local.segment.start * _transform.worldMat;
    _world.segment.end   = _local.segment.end * _transform.worldMat;
    // 半径はスカラーなので軸ごとに異なるスケールをそのまま反映することはできない。
    // 非一様スケールがかかっていても判定が小さくなりすぎないよう、各軸スケールの最大値を採用する
    Vec3f scale    = _transform.GetWorldScale();
    float maxScale = std::max({scale[X], scale[Y], scale[Z]});
    _world.radius  = _local.radius * maxScale;
}

/// <summary>
/// ワールド空間のAABBを取得する
/// </summary>
/// <returns>広域フェーズ（空間ハッシュ登録など）に使うワールド空間の外接AABB</returns>
Bounds::AABB CapsuleCollider::ToWorldAABB() const {
    Vec3f minPt, maxPt;
    const Vec3f& start = worldShape_.segment.start;
    const Vec3f& end   = worldShape_.segment.end;
    float r            = worldShape_.radius;

    // start/endを結ぶ線分の軸平行境界を求め、半径分だけ外側に拡張する
    for (int i = 0; i < 3; ++i) {
        minPt[i] = std::min(start[i], end[i]) - r;
        maxPt[i] = std::max(start[i], end[i]) + r;
    }

    Vec3f center   = (minPt + maxPt) * 0.5f;
    Vec3f halfSize = (maxPt - minPt) * 0.5f;
    return Bounds::AABB(center, halfSize);
}

} // namespace OriGine

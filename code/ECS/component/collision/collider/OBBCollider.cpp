#include "OBBCollider.h"

using namespace OriGine;

/// <summary>
/// OBBColliderの状態をJSONへ書き出す
/// </summary>
/// <param name="_json">書き込み先のJSON</param>
/// <param name="_o">シリアライズ対象のOBBCollider</param>
void OriGine::to_json(nlohmann::json& _json, const OBBCollider& _o) {
    to_json(_json, static_cast<const ICollider&>(_o));
    _json["center"]       = _o.GetLocalCenter();
    _json["halfSize"]     = _o.GetLocalHalfSize();
    _json["orientations"] = _o.shape_.orientations_.rot;
    _json["transform"]    = _o.transform_;
}
/// <summary>
/// JSONからOBBColliderの状態を復元する
/// </summary>
/// <param name="_json">読み込み元のJSON</param>
/// <param name="_o">復元先のOBBCollider</param>
void OriGine::from_json(const nlohmann::json& _json, OBBCollider& _o) {
    from_json(_json, static_cast<ICollider&>(_o));
    // 古いセーブデータにキーが無い場合でも読み込みが壊れないよう、
    // 各項目の存在をcontains()で確認してから取得する
    if (_json.contains("center")) {
        _json.at("center").get_to(_o.shape_.center_);
    }
    if (_json.contains("halfSize")) {
        _json.at("halfSize").get_to(_o.shape_.halfSize_);
    }
    if (_json.contains("orientations")) {
        _json.at("orientations").get_to(_o.shape_.orientations_.rot);
        _o.shape_.orientations_.UpdateAxes();
    }
    if (_json.contains("transform")) {
        _json.at("transform").get_to(_o.transform_);
    }
}

/// <summary>
/// デバッグ用GUIでOBB形状とTransformのパラメータを編集する
/// </summary>
/// <param name="_scene">対象シーン</param>
/// <param name="_handle">対象エンティティ</param>
/// <param name="_parentLabel">ImGuiのID衝突を避けるための親ラベル</param>
void OBBCollider::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] const EntityHandle& _handle, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef ORIGINE_EDITOR_ENABLED
    // エディタ専用のUIコードなので、リリースビルドには含めない

    ICollider::Edit(_scene, _handle, _parentLabel);

    std::string label = "OBB##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        DragGuiVectorCommand<3, float>("Center##" + _parentLabel, shape_.center_, 0.01f);
        DragGuiVectorCommand<3, float>("HalfSize##" + _parentLabel, shape_.halfSize_, 0.01f);

        ImGui::Spacing();

        DragGuiVectorCommand<4, float>("Rotation##" + _parentLabel, shape_.orientations_.rot, 0.01f);
        shape_.orientations_.UpdateAxes();

        ImGui::Text("Axes:");
        ImGui::Text("X: %.2f, %.2f, %.2f", shape_.orientations_.axis[X][X], shape_.orientations_.axis[X][Y], shape_.orientations_.axis[X][Z]);
        ImGui::Text("Y: %.2f, %.2f, %.2f", shape_.orientations_.axis[Y][X], shape_.orientations_.axis[Y][Y], shape_.orientations_.axis[Y][Z]);
        ImGui::Text("Z: %.2f, %.2f, %.2f", shape_.orientations_.axis[Z][X], shape_.orientations_.axis[Z][Y], shape_.orientations_.axis[Z][Z]);

        ImGui::TreePop();
    }

    label = "Transform##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        transform_.Edit(_scene, _handle, _parentLabel);
        ImGui::TreePop();
    }

#endif
};

/// <summary>
/// ローカル形状(_local)とTransformの現在値から、ワールド空間のOBB(_world)を再計算する。
/// Collider&lt;Bounds::OBB&gt;::CalculateWorldShape()から形状型で選ばれて呼ばれる(Phase 3 3B)。
/// </summary>
void OriGine::Bounds::CalculateWorldShape(const OriGine::Bounds::OBB& _local, OriGine::Bounds::OBB& _world, const Transform& _transform) {
    // 中心は平行移動を含むワールド行列全体で変換する
    _world.center_           = _local.center_ * _transform.worldMat;
    // ハーフサイズは向きを持たない「大きさ」なので、ワールドスケールのみを軸ごとに乗算する
    _world.halfSize_         = _local.halfSize_ * _transform.GetWorldScale();
    // ローカルの回転にワールド回転を合成してから、各軸ベクトルを再計算する
    _world.orientations_.rot = _local.orientations_.rot * _transform.CalculateWorldRotate();
    _world.orientations_.UpdateAxes();
}

/// <summary>
/// ワールド空間のAABBを取得する
/// </summary>
/// <returns>広域フェーズ（空間ハッシュ登録など）に使う、OBBを軸並行境界に投影したAABB</returns>
Bounds::AABB OBBCollider::ToWorldAABB() const {
    // OBBの各軸(axes)をワールド軸(X/Y/Z)へ投影し、その絶対値にハーフサイズを掛けて足し合わせることで
    // OBBを内包する最小のAABBの半径（extent）を求める
    const auto& axes = worldShape_.orientations_.axis;
    const auto& half = worldShape_.halfSize_;

    Vec3f extent;
    for (int i = 0; i < 3; ++i) {
        extent[i] = std::abs(axes[X][i]) * half[X] + std::abs(axes[Y][i]) * half[Y] + std::abs(axes[Z][i]) * half[Z];
    }

    return Bounds::AABB(worldShape_.center_, extent);
}

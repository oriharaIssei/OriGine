#include "SphereCollider.h"

/// stl
#include <algorithm>

using namespace OriGine;

/// <summary>
/// SphereColliderの状態をJSONへ書き出す
/// </summary>
/// <param name="_json">書き込み先のJSON</param>
/// <param name="_s">シリアライズ対象のSphereCollider</param>
void OriGine::to_json(nlohmann::json& _json, const SphereCollider& _s) {
    to_json(_json, static_cast<const ICollider&>(_s));
    _json["center"]    = _s.GetLocalCenter();
    _json["radius"]    = _s.GetLocalRadius();
    _json["transform"] = _s.transform_;
}
/// <summary>
/// JSONからSphereColliderの状態を復元する
/// </summary>
/// <param name="_json">読み込み元のJSON</param>
/// <param name="_s">復元先のSphereCollider</param>
void OriGine::from_json(const nlohmann::json& _json, SphereCollider& _s) {
    from_json(_json, static_cast<ICollider&>(_s));
    // 古いセーブデータにキーが無い場合でも読み込みが壊れないよう、
    // 各項目の存在をcontains()で確認してから取得する
    if (_json.contains("center")) {
        _json.at("center").get_to(_s.shape_.center_);
    }
    if (_json.contains("radius")) {
        _json.at("radius").get_to(_s.shape_.radius_);
    }
    if (_json.contains("transform")) {
        _json.at("transform").get_to(_s.transform_);
    }
}

/// <summary>
/// デバッグ用GUIでSphere形状とTransformのパラメータを編集する
/// </summary>
/// <param name="_scene">対象シーン</param>
/// <param name="_handle">対象エンティティ</param>
/// <param name="_parentLabel">ImGuiのID衝突を避けるための親ラベル</param>
void SphereCollider::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] const EntityHandle& _handle, [[maybe_unused]] const std::string& _parentLabel) {

#ifdef ORIGINE_EDITOR_ENABLED
    // エディタ専用のUIコードなので、リリースビルドには含めない

    ICollider::Edit(_scene, _handle, _parentLabel);

    std::string label = "Sphere##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        DragGuiVectorCommand<3, float>("Center##" + _parentLabel, shape_.center_, 0.01f);
        DragGuiCommand<float>("Radius##" + _parentLabel, shape_.radius_, 0.01f);
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
/// ローカル形状(_local)とTransformの現在値から、ワールド空間のSphere(_world)を再計算する。
/// Collider&lt;Bounds::Sphere&gt;::CalculateWorldShape()から形状型で選ばれて呼ばれる
/// (旧: SphereCollider::CalculateWorldShape()の中身そのまま。transform_.UpdateMatrix()の
/// 呼び出しだけはCollider側に共通化したのでここには無い)。
/// </summary>
void OriGine::Bounds::CalculateWorldShape(const OriGine::Bounds::Sphere& _local, OriGine::Bounds::Sphere& _world, const Transform& _transform) {
    // 中心は平行移動を含むワールド行列全体で変換する
    _world.center_ = _local.center_ * _transform.worldMat;
    // 半径はスカラーなので行列を掛けられない。非一様スケールがかかっていても球の形を保てるよう、
    // CapsuleColliderと同じく各軸スケールの最大値を採用する(最小値だと本来の形状より判定が小さくなる)
    Vec3f scale    = _transform.GetWorldScale();
    float maxScale = std::max({scale[X], scale[Y], scale[Z]});
    _world.radius_ = _local.radius_ * maxScale;
}

/// <summary>
/// ワールド空間のAABBを取得する
/// </summary>
/// <returns>広域フェーズ（空間ハッシュ登録など）に使うワールド空間の外接AABB</returns>
Bounds::AABB SphereCollider::ToWorldAABB() const {
    // 球は等方形状なので、半径をそのまま各軸のハーフサイズとして使える
    Vec3f halfSize(worldShape_.radius_, worldShape_.radius_, worldShape_.radius_);
    return Bounds::AABB(worldShape_.center_, halfSize);
}

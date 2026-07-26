#include "SegmentCollider.h"

namespace OriGine {

/// <summary>
/// SegmentColliderの状態をJSONへ書き出す
/// </summary>
/// <param name="_json">書き込み先のJSON</param>
/// <param name="_s">シリアライズ対象のSegmentCollider</param>
void to_json(nlohmann::json& _json, const SegmentCollider& _s) {
    to_json(_json, static_cast<const ICollider&>(_s));
    _json["start"]     = _s.shape_.start;
    _json["end"]       = _s.shape_.end;
    _json["transform"] = _s.transform_;
}

/// <summary>
/// JSONからSegmentColliderの状態を復元する
/// </summary>
/// <param name="_json">読み込み元のJSON</param>
/// <param name="_s">復元先のSegmentCollider</param>
void from_json(const nlohmann::json& _json, SegmentCollider& _s) {
    from_json(_json, static_cast<ICollider&>(_s));
    // 古いセーブデータにキーが無い場合でも読み込みが壊れないよう、
    // 各項目の存在をcontains()で確認してから取得する
    if (_json.contains("start")) {
        _json.at("start").get_to(_s.shape_.start);
    }
    if (_json.contains("end")) {
        _json.at("end").get_to(_s.shape_.end);
    }
    if (_json.contains("transform")) {
        _json.at("transform").get_to(_s.transform_);
    }
}

/// <summary>
/// デバッグ用GUIでSegment形状とTransformのパラメータを編集する
/// </summary>
/// <param name="_scene">対象シーン</param>
/// <param name="_handle">対象エンティティ</param>
/// <param name="_parentLabel">ImGuiのID衝突を避けるための親ラベル</param>
void SegmentCollider::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] const EntityHandle& _handle, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG
    // エディタ専用のUIコードなので、リリースビルドには含めない

    ICollider::Edit(_scene, _handle, _parentLabel);

    std::string label = "Segment##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        DragGuiVectorCommand<3, float>("Start##" + _parentLabel, this->shape_.start, 0.01f);
        DragGuiVectorCommand<3, float>("End##" + _parentLabel, this->shape_.end, 0.01f);
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
/// ローカル形状(shape_)とTransformの現在値から、ワールド空間のSegment(worldShape_)を再計算する
/// </summary>
void SegmentCollider::CalculateWorldShape() {
    transform_.UpdateMatrix();
    // start/endはどちらも位置なので、平行移動を含むワールド行列全体で変換する
    this->worldShape_.start = shape_.start * transform_.worldMat;
    this->worldShape_.end   = shape_.end * transform_.worldMat;
}

/// <summary>
/// ワールド空間のAABBを取得する
/// </summary>
/// <returns>広域フェーズ（空間ハッシュ登録など）に使うワールド空間の外接AABB</returns>
Bounds::AABB SegmentCollider::ToWorldAABB() const {
    Vec3f minPt, maxPt;
    const Vec3f& start = worldShape_.start;
    const Vec3f& end   = worldShape_.end;

    for (int i = 0; i < 3; ++i) {
        minPt[i] = std::min(start[i], end[i]);
        maxPt[i] = std::max(start[i], end[i]);
    }

    Vec3f center   = (minPt + maxPt) * 0.5f;
    Vec3f halfSize = (maxPt - minPt) * 0.5f;

    // 線分は太さを持たないため、軸によってはhalfSizeが0になり得る。
    // 空間ハッシュや交差判定でゼロ幅のAABBを扱わずに済むよう、各軸に最小サイズを保証する
    const float kMinSize = 0.001f;
    for (int i = 0; i < 3; ++i) {
        if (halfSize[i] < kMinSize)
            halfSize[i] = kMinSize;
    }

    return Bounds::AABB(center, halfSize);
}

} // namespace OriGine

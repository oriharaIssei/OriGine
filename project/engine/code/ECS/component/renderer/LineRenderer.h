#pragma once

#include "MeshRenderer.h"

namespace OriGine {

///< summary>
/// ライン描画コンポーネント
///</summary>
class LineRenderer
    : public MeshRenderer<Mesh<ColorVertexData>, ColorVertexData> {
    friend void to_json(nlohmann::json& _j, const LineRenderer& _comp);
    friend void from_json(const nlohmann::json& _j, LineRenderer& _comp);

public:
    LineRenderer();
    LineRenderer(const std::vector<Mesh<ColorVertexData>>& _meshGroup);
    LineRenderer(const std::shared_ptr<std::vector<Mesh<ColorVertexData>>>& _meshGroup);
    ~LineRenderer();
    ///< summary>
    /// 初期化
    ///</summary>
    void Initialize(Scene* _scene, EntityHandle _owner) override;

    void Edit(Scene* _scene, EntityHandle _entity, const std::string& _parentLabel) override;
    void Finalize() override;

private:
    IConstantBuffer<Transform> transformBuff_;
    bool lineIsStrip_ = false;

public:
    //------------------------------ Transform ------------------------------//
    const Transform& GetTransform() const {
        return transformBuff_.openData_;
    }
    void SetTransform(const Transform& _transform) {
        transformBuff_.openData_ = _transform;
    }
    /// <summary>
    /// 親Transformを設定
    /// </summary>
    /// <param name="_parent"></param>
    void SetParentTransform(Transform* _parent) {
        transformBuff_.openData_.parent = _parent;
    }
    IConstantBuffer<Transform>& GetTransformBuff() {
        return transformBuff_;
    }

    //------------------------------ LineIsStrip ------------------------------//
    bool isLineStrip() const {
        return lineIsStrip_;
    }
    void SetLineStrip(bool _isStrip) {
        lineIsStrip_ = _isStrip;
    }
};

} // namespace OriGine

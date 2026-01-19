#pragma once

#include "component/IComponent.h"

/// engine
// directX12
#include "directX12/buffer/IConstantBuffer.h"

/// math
#include "math/Vector4.h"

namespace OriGine {

/// <summary>
/// アウトラインエフェクト用パラメータデータ
/// </summary>
struct OutlineParamData {
    float outlineWidth = 0.3f; // アウトラインの太さ
    Vec4f outlineColor = kWhite; // アウトラインの色

    struct ConstantBuffer {
        float outlineWidth;
        float padding[3]; // 16バイトアライメント用パディング
        Vec4f outlineColor;

        ConstantBuffer& operator=(const OutlineParamData& _data) {
            outlineWidth = _data.outlineWidth;
            outlineColor = _data.outlineColor;
            return *this;
        }
    };
};

/// <summary>
/// アウトラインエフェクトコンポーネント
/// </summary>
struct OutlineComponent
    : public IComponent {
    friend void to_json(nlohmann::json& _j, const OutlineComponent& _comp);
    friend void from_json(const nlohmann::json& _j, OutlineComponent& _comp);

public:
    OutlineComponent();
    ~OutlineComponent() override;

    void Initialize(Scene* _scene, EntityHandle _owner) override;
    void Finalize() override;
    void Edit(Scene* _scene, EntityHandle _owner, const std::string& _parentLabel) override;

public:
    bool isActive = false; // エフェクトが有効かどうか
    ComponentHandle usingMaterialHandle; // 使用するマテリアルコンポーネントハンドル(未設定時はデフォルトマテリアルを使用)
    IConstantBuffer<OutlineParamData> paramData; // アウトラインエフェクト用パラメータデータ
};

} // namespace OriGine

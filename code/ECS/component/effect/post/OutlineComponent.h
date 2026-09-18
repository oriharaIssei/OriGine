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
    ORIGINE_COMPONENT();

    OutlineComponent();
    ~OutlineComponent();

    void Initialize(Scene* _scene, const EntityHandle& _owner);
    void Finalize();
    void Edit(Scene* _scene, const EntityHandle& _owner, const std::string& _parentLabel);

public:
    bool isActive = false; // エフェクトが有効かどうか
    ComponentHandle usingMaterialHandle; // 使用するマテリアルコンポーネントハンドル(未設定時はデフォルトマテリアルを使用)
    // このフィールド自体はキー "paramData" では保存されない。実際の to_json は
    // paramData.openData_ の中の outlineWidth/outlineColor を個別キーで書く
    // (ネストしたメンバの展開は 3D のシリアライズ移行までディスクリプタの対象外)。
    ORIGINE_FIELD(no_save);
    IConstantBuffer<OutlineParamData> paramData; // アウトラインエフェクト用パラメータデータ
};

} // namespace OriGine

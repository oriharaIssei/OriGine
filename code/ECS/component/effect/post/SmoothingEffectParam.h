#pragma once

#include "component/IComponent.h"

/// engine
// directX12
#include "directX12/buffer/IConstantBuffer.h"

/// math
#include "math/Vector2.h"

namespace OriGine {

/// <summary>
/// Smoothing に 使用する BoxFilter のサイズ
/// </summary>
struct BoxFilterSize {
    ORIGINE_STRUCT();

    Vec2f size = Vec2f();

public:
    struct ConstantBuffer {
        Vec2f boxSize;
        ConstantBuffer& operator=(const BoxFilterSize& _size) {
            boxSize = _size.size;
            return *this;
        }
    };
};

/// <remarks>
/// D-1: 保存対象フィールドだけを見ると表経由にできる型だが、kUsesDescriptorSerialization は
/// 導入していない。to_json/from_json が `namespace OriGine` の外(グローバル名前空間)に
/// 定義されていて ADL 経由のシリアライズが機能しない既知のバグが以前あったが、
/// 2026-09-22 に SmoothingEffectParam.cpp 側を修正済み(friend 宣言と同じ名前空間に移動)。
/// 表経由への切り替えでこのバグを迂回して「直って見える」ことを避けるため、バグ修正後も
/// 手書きの to_json/from_json のまま残している(表経由への移行自体は別スコープ)。
/// </remarks>
struct SmoothingEffectParam
    : public IComponent {
    friend void to_json(nlohmann::json& _j, const SmoothingEffectParam& _comp);
    friend void from_json(const nlohmann::json& _j, SmoothingEffectParam& _comp);

public:
    ORIGINE_COMPONENT();

    SmoothingEffectParam()           = default;
    ~SmoothingEffectParam() = default;

    void Initialize(Scene* _scene, const EntityHandle& _entity);
    void Edit(Scene* _scene, const EntityHandle& _entity, const std::string& _parentLabel);
    void Finalize();

public:
    bool isActive_ = true;
    // このフィールド自体はキー "boxFilterSize" では保存されない。実際の to_json は
    // boxFilterSize_.openData_.size を書く(ネストしたメンバの展開は 3D まで対象外)。
    ORIGINE_FIELD(no_save);
    IConstantBuffer<BoxFilterSize> boxFilterSize_;
};

} // namespace OriGine

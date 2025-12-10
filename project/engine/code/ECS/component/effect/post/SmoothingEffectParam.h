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

struct SmoothingEffectParam
    : public IComponent {
    friend void to_json(nlohmann::json& j, const SmoothingEffectParam& c);
    friend void from_json(const nlohmann::json& j, SmoothingEffectParam& c);

public:
    SmoothingEffectParam()           = default;
    ~SmoothingEffectParam() override = default;

    void Initialize(Entity* _entity) override;
    void Edit(Scene* _scene, Entity* _entity, const std::string& _parentLabel) override;
    void Finalize() override;

public:
    bool isActive_ = true;
    IConstantBuffer<BoxFilterSize> boxFilterSize_;
};

} // namespace OriGine

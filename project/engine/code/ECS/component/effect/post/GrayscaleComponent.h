#pragma once

#include "component/IComponent.h"

/// engine
// directX12
#include "directX12/buffer/IConstantBuffer.h"

namespace OriGine {

/// <summary>
/// グレースケールエフェクトのパラメータデータ
/// </summary>
struct GrayscaleParamData {
    float amount{0.0f}; // グレースケールの量 (0.0f = なし, 1.0f = 完全なグレースケール)

    struct ConstantBuffer {
        float amount; // グレースケールの量
        float padding[3]; // パディング（16バイトアラインメントのため）

        ConstantBuffer& operator=(const GrayscaleParamData& _data) {
            amount = _data.amount;
            return *this;
        }
    };
};

/// <summary>
/// グレースケールエフェクトコンポーネント
/// </summary>
class GrayscaleComponent
    : public IComponent {
    friend void to_json(nlohmann::json& _j, const GrayscaleComponent& _component);
    friend void from_json(const nlohmann::json& _j, GrayscaleComponent& _component);

public:
    GrayscaleComponent()           = default;
    ~GrayscaleComponent() override = default;

    void Initialize(Scene* _scene, EntityHandle _owner) override;
    void Finalize() override;

    void Edit(Scene* _scene, EntityHandle _owner, const std::string& _parentLabel) override;

private:
    bool isEnabled_{false};
    IConstantBuffer<GrayscaleParamData> constantBuffer_;

public:
    bool IsEnabled() const {
        return isEnabled_;
    }
    void SetIsEnabled(bool _isEnabled) {
        isEnabled_ = _isEnabled;
    };

    float GetAmount() const {
        return constantBuffer_.openData_.amount;
    }
    void SetAmount(float _amount) {
        constantBuffer_.openData_.amount = _amount;
    }

    const IConstantBuffer<GrayscaleParamData>& GetConstantBuffer() const {
        return constantBuffer_;
    }
};
} // namespace OriGine

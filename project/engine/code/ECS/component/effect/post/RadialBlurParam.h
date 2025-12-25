#pragma once

/// parent
#include "component/IComponent.h"

/// engine
// directX12
#include "directX12/buffer/IConstantBuffer.h"

/// math
#include "math/Vector2.h"

namespace OriGine {

/// <summary>
/// RadialBlur の パラメーター情報(GPUに送るもの)
/// </summary>
class RadialBlurBufferParam {
public:
    RadialBlurBufferParam() {}
    ~RadialBlurBufferParam() {}

public:
    class ConstantBuffer {
    public:
        ConstantBuffer()  = default;
        ~ConstantBuffer() = default;

        ConstantBuffer& operator=(const RadialBlurBufferParam& other) {
            center = other.center_;
            width  = other.width_;
            return *this;
        }

    private:
        Vec2f center = {0.0f, 0.0f}; // Center of the radial blur effect
        float width  = 0.0f; // Width of the radial blur effect
    };

public:
    Vec2f center_ = {0.0f, 0.0f};
    float width_  = 0.0f;
};

/// <summary>
/// RadialBlurのパラメーター
/// </summary>
class RadialBlurParam
    : public IComponent {
public:
    friend void to_json(nlohmann::json& j, const RadialBlurParam& param);
    friend void from_json(const nlohmann::json& j, RadialBlurParam& param);

public:
    RadialBlurParam();
    ~RadialBlurParam();

    void Initialize(Scene* _scene, EntityHandle _entity);

    void Edit(Scene* _scene, EntityHandle _handle, const std::string& _parentLabel);

    void Finalize();

private:
    bool isActive_ = false;
    IConstantBuffer<RadialBlurBufferParam> constantBuffer_;

public:
    bool IsActive() const { return isActive_; }
    void Play();
    void Stop();

    const Vec2f& GetCenter() const { return constantBuffer_.openData_.center_; }
    void SetCenter(const Vec2f& center) { constantBuffer_.openData_.center_ = center; }
    float GetWidth() const { return constantBuffer_.openData_.width_; }
    void SetWidth(float width) { constantBuffer_.openData_.width_ = width; }

    const IConstantBuffer<RadialBlurBufferParam>& GetConstantBuffer() const {
        return constantBuffer_;
    }
};

} // namespace OriGine

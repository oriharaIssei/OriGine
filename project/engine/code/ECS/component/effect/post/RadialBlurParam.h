#pragma once

/// parent
#include "component/IComponent.h"

/// engine
// directX12
#include "directX12/IConstantBuffer.h"

/// math
#include "math/Vector2.h"

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

class RadialBlurParam
    : public IComponent {
    friend void to_json(nlohmann::json& j, const RadialBlurParam& param);
    friend void from_json(const nlohmann::json& j, RadialBlurParam& param);

public:
    RadialBlurParam();
    ~RadialBlurParam();

    void Initialize(GameEntity* _entity);

    void Edit(Scene* _scene,GameEntity* _entity,[[maybe_unused]] const std::string& _parentLabel);

    void Finalize();

private:
    bool isActive_        = false;
    IConstantBuffer<RadialBlurBufferParam> constantBuffer_;

public:
    bool isActive() const { return isActive_; }
    void Play();
    void Stop();

    const Vec2f& getCenter() const { return constantBuffer_.openData_.center_; }
    void setCenter(const Vec2f& center) { constantBuffer_.openData_.center_ = center; }
    float getWidth() const { return constantBuffer_.openData_.width_; }
    void setWidth(float width) { constantBuffer_.openData_.width_ = width; }

    const IConstantBuffer<RadialBlurBufferParam>& getConstantBuffer() const {
        return constantBuffer_;
    }
};

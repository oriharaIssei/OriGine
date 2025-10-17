#pragma once

#include "component/renderer/primitive/base/IPrimitive.h"

namespace Primitive {

/// <summary>
/// Plane(面)のPrimitiveクラス
/// </summary>
class Plane
    : public IPrimitive {
public:
    Plane() : IPrimitive(PrimitiveType::Plane) {}
    ~Plane() override {}

    void createMesh(TextureMesh* _mesh) override;

private:
    Vec2f size_   = {1.0f, 1.0f};
    Vec2f uv_     = {1.0f, 1.0f};
    Vec3f normal_ = {0.0f, 0.0f, 1.0f};

public: // accessor
    const Vec2f& getSize() const {
        return size_;
    }
    void setSize(const Vec2f& _size) {
        size_ = _size;
    }
    const Vec2f& getUV() const {
        return uv_;
    }
    void setUV(const Vec2f& _uv) {
        uv_ = _uv;
    }
    const Vec3f& getNormal() const {
        return normal_;
    }
    void setNormal(const Vec3f& _normal) {
        normal_ = _normal;
    }
};

}

#pragma once

#include "component/renderer/primitive/base/IPrimitive.h"

namespace Primitive {

/// <summary>
/// Sphere(球)のPrimitiveクラス
/// </summary>
class Sphere
    : public IPrimitive {
public:
    Sphere() : IPrimitive(PrimitiveType::Sphere) {}
    ~Sphere() override {}
    void createMesh(TextureMesh* _mesh) override;

private:
    float radius_               = 1.0f; // 半径
    uint32_t divisionLatitude_  = 8; // 緯度分割数
    uint32_t divisionLongitude_ = 16; // 経度分割数
public:
    float getRadius() const {
        return radius_;
    }
    void setRadius(float _radius) {
        radius_ = _radius;
    }
    uint32_t getDivisionLatitude() const {
        return divisionLatitude_;
    }
    void setDivisionLatitude(uint32_t _division) {
        divisionLatitude_ = _division;
    }
    uint32_t getDivisionLongitude() const {
        return divisionLongitude_;
    }
    void setDivisionLongitude(uint32_t _division) {
        divisionLongitude_ = _division;
    }
};

} // namespace Primitive

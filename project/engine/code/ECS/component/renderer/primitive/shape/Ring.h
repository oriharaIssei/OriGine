#pragma once

#include "component/renderer/primitive/base/IPrimitive.h"

namespace Primitive {

/// <summary>
/// Ring(環)のPrimitiveクラス
/// </summary>
class Ring
    : public IPrimitive {
public:
    Ring() : IPrimitive(PrimitiveType::Ring) {}
    ~Ring() override {}

    void createMesh(TextureMesh* _mesh) override;

private:
    uint32_t division_ = 16;

    float outerRadius_ = 1.f;
    float innerRadius_ = 0.5f;

public:
    float getOuterRadius() const {
        return outerRadius_;
    }
    void setOuterRadius(float _outerRadius) {
        outerRadius_ = _outerRadius;
    }
    float getInnerRadius() const {
        return innerRadius_;
    }
    void setInnerRadius(float _innerRadius) {
        innerRadius_ = _innerRadius;
    }
    uint32_t getDivision() const {
        return division_;
    }
    void setDivision(uint32_t _division) {
        division_ = _division;
    }
};

} // namespace Primitive

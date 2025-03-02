#pragma once

#include <d3d12.h>
#include <wrl.h>

#include <string>

#include "lib/globalVariables/SerializedField.h"

#include "math/Vector3.h"
#include "math/Vector4.h"

struct PointLight {
public:
    PointLight(const std::string& scene, int32_t index) : color{scene, "PointLight" + std::to_string(index), "color"},
                                                          pos{scene, "PointLight" + std::to_string(index), "pos"},
                                                          intensity{scene, "PointLight" + std::to_string(index), "intensity"},
                                                          radius{scene, "PointLight" + std::to_string(index), "radius"},
                                                          decay{scene, "PointLight" + std::to_string(index), "decay"} {}

    ~PointLight() {}

    SerializedField<Vec3f> color;
    SerializedField<Vec3f> pos;
    SerializedField<float> intensity;
    SerializedField<float> radius;
    SerializedField<float> decay;

public:
    struct ConstantBuffer {
        Vec3f color; // 12 bytes
        float intensity; // 4 bytes to align to 16 bytes
        Vec3f pos; // 12 bytes
        float radius; // 4 bytes
        float decay; // 4 bytes
        float padding[3];
        ConstantBuffer& operator=(const PointLight& light) {
            color     = light.color;
            pos       = light.pos;
            intensity = light.intensity;
            radius    = light.radius;
            decay     = light.decay;
            return *this;
        }
    };
};

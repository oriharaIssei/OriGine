#pragma once

#include <d3d12.h>
#include <wrl.h>

#include <string>

#include "globalVariables/SerializedField.h"

#include "math/Vector3.h"
#include "math/Vector4.h"

struct DirectionalLight {
public:
    DirectionalLight(const std::string& scene, int32_t index) : color{scene, "DirectionalLight" + std::to_string(index), "color"},
                                                                direction{scene, "DirectionalLight" + std::to_string(index), "direction"},
                                                                intensity{scene, "DirectionalLight" + std::to_string(index), "intensity"} {}

    ~DirectionalLight() {}

    SerializedField<Vec3f> color;
    SerializedField<Vec3f> direction;
    SerializedField<float> intensity;

public:
    struct ConstantBuffer {
        Vec3f color; // 12 bytes
        float intensity; // 4 bytes
        Vec3f direction; // 12 bytes
        ConstantBuffer& operator=(const DirectionalLight& light) {
            color     = light.color;
            direction = light.direction;
            intensity = light.intensity;
            return *this;
        }
    };
};

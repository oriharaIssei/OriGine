#pragma once

/// microsoft
#include <wrl.h>
/// dx12
#include <d3d12.h>
/// stl
#include <array>
#include <functional>
#include <string>

/// engine
#include "component/IComponent.h"

/// math
#include "Matrix4x4.h"
#include "Quaternion.h"

struct Transform
    : public IComponent {
public:
    Transform() {}
    Transform(const Vec3f &_scale, const Quaternion &_rotate, const Vec3f &_translate)
        : scale(_scale), rotate(_rotate), translate(_translate), worldMat(MakeMatrix::Identity()) {}
    ~Transform() {}

    void Init() override;
    void Update() override;

    /// <summary>
    /// ImGuiでの要素表示関数(Matrixの更新はしない)
    /// </summary>
    void Debug(const std::string &transformName);

    Vec3f scale = {1.0f, 1.0f, 1.0f};
    Quaternion rotate = {0.0f, 0.0f, 0.0f, 1.0f};
    Vec3f translate = {0.0f, 0.0f, 0.0f};
    Matrix4x4 worldMat;

    Transform *parent = nullptr;

public:
    struct ConstantBuffer {
        Matrix4x4 world;
        ConstantBuffer &operator=(const Transform &transform) {
            world = transform.worldMat;
            return *this;
        }
    };
};

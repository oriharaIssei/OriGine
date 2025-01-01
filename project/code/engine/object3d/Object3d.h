#pragma once

#include "directX12/IConstantBuffer.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"
#include "material/Material.h"
#include "model/Model.h"
#include "transform/CameraTransform.h"
#include "transform/Transform.h"

#include <memory>
#include <vector>

class Object3d {
public:
    static void PreDraw();

    static void setBlendMode(BlendMode blend) { currentBlend_ = blend; }

    void Init(const std::string& directoryPath, const std::string& filename);

private:
    static BlendMode currentBlend_;

public:
    Object3d() = default;
    ~Object3d() {}

    Transform transform_;

    void UpdateTransform();

    void Draw();

private:
    void NotDraw() {}

    void DrawThis();

private:
    std::unique_ptr<Model> data_;

    std::array<std::function<void()>, 2> drawFuncTable_ = {
        [this]() {
            NotDraw();
        },
        [this]() {
            DrawThis();
        }};

public:
    const Model* getModel() const { return data_.get(); }
    void setMaterial(IConstantBuffer<Material>* material, uint32_t index = 0);
};

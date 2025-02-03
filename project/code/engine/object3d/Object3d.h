#pragma once

///stl
#include <functional>
#include <memory>
//container
#include <array>
#include <vector>
///engine
//dx12Object
#include "directX12/IConstantBuffer.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"
//assets
#include "material/Material.h"
#include "model/Model.h"
//transform
#include "transform/CameraTransform.h"
#include "transform/Transform.h"

class Object3d {
public:
    static void PreDraw();

    static void setBlendMode(BlendMode blend);

    void Init(const std::string& directoryPath, const std::string& filename);

private:
    static BlendMode currentBlend_;

public:
    Object3d();
    ~Object3d();

    Transform transform_;

    void UpdateTransform();

    void Draw();

private:
    void NotDraw() {}

    void DrawThis();

private:
    std::shared_ptr<Model> data_;

    std::array<std::function<void()>, 2> drawFuncTable_ = {
        [this]() {
            NotDraw();
        },
        [this]() {
            DrawThis();
        }};

public:
    const Model* getModel() const { return data_.get(); }
    Model* getModel() { return data_.get(); }

    void setMaterial(IConstantBuffer<Material>* material, uint32_t index = 0);
};

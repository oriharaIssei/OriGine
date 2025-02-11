#pragma once

/// stl
#include <memory>
#include <vector>

/// engine
// assets
#include "material/Material.h"
#include "model/Model.h"
// dxObject
#include "directX12/IConstantBuffer.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"
#include "transform/CameraTransform.h"
#include "transform/Transform.h"

// hostObject
#include "component/renderer/IRendererComponent.h"

class ModelRenderer
    : public IRendererComponent {
public:
    //------------------------- static member -------------------------//
    static void PreDraw();

    static void setBlendMode(BlendMode blend);

private:
    static BlendMode currentBlend_;

public:
    //------------------------- member -------------------------//
    ModelRenderer(const std::string& directoryPath, const std::string& filename);
    ~ModelRenderer();

    void Init() override;
    void Update() override;
    void Render() override;

    Transform transform_;

private:
    //------------------------- DrawFunction -------------------------//
    void NotDraw() {} // < 何もしない. 読み込み中に描画されないようにするための関数
    void DrawThis(); //  < モデルを描画する関数

private:
    std::string& directoryPath_;

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
    Model* getModel() { return data_.get(); }

    void setMaterial(IConstantBuffer<Material>* material, uint32_t index = 0);
};

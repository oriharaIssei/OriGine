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

class AnimationObject3d {
public:
    static std::unique_ptr<AnimationObject3d> Create(const std::string& _modelDirectoryPath, const std::string& _modelFilename);

    static std::unique_ptr<AnimationObject3d> Create(const std::string& _modelDirectoryPath, const std::string& _modelFilename, const std::string& _animationDirectoryPath, const std::string& _animationFilename);
    // まだ Object3d と 同じで良い
    //	static void PreDraw();

private:
    static BlendMode currentBlend_;

public:
    AnimationObject3d() = default;
    ~AnimationObject3d() {}

    Transform transform_;

    void Update(float deltaTime);
    void Draw();

private:
    void NotDraw() {}

    void DrawThis();

private:
    std::array<std::function<void()>, 2> drawFuncTable_ = {
        [this]() {
            NotDraw();
        },
        [this]() {
            DrawThis();
        }};

    std::unique_ptr<Model> data_;
    std::unique_ptr<Animation> animation_;

public:
    const Model* getModel() const { return data_.get(); }
    Model* getModel() { return data_.get(); }
    void setModel(std::unique_ptr<Model>&& model){ data_ = std::move(model); }
    void setModel(const std::string& directory,const std::string& filename);

    const Animation* getAnimation() const { return animation_.get(); }
    Animation* getAnimation() { return animation_.get(); }

    void setMaterial(IConstantBuffer<Material>* material, uint32_t index = 0);
};

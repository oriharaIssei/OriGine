#pragma once

#include "component/material/Material.h"
#include "component/transform/Transform.h"
#include "directX12/IConstantBuffer.h"

#include <memory>
#include <string>
#include <vector>

struct Model;
struct Animation;
struct AnimationSetting;

class AnimationObject3d {
public:
    // Object3d と 同じ Pipeline を使うため 必要なし
    //	static void PreDraw();

public:
    AnimationObject3d();
    ~AnimationObject3d();

    void Initialize(
        const std::string& _directoryPath,
        const std::string& _filename);
    void Initialize(const AnimationSetting& _animationSetting);
    void Initialize(
        const std::string& _modelDirectoryPath,
        const std::string& _modelFilename,
        const std::string& _animationDirectoryPath,
        const std::string& _animationFilename);

    void Update(float deltaTime);
    void Draw();

    Transform transform_;

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

    std::shared_ptr<Model> model_;
    std::string currentAnimationName_;
    std::unique_ptr<Animation> animation_;

    std::unique_ptr<Animation> toNextAnimation_;
    std::unique_ptr<Animation> nextAnimation_;

public:
    const Model* getModel() const;
    Model* getModel();
    void setModel(std::shared_ptr<Model> model);
    void setModel(const std::string& directory, const std::string& filename);

    const Animation* getAnimation() const;
    Animation* getAnimation();
    void setAnimation(const std::string& directory, const std::string& filename);
    void setAnimation(std::unique_ptr<Animation>& animation);
    void setNextAnimation(std::unique_ptr<Animation>& animation, const std::string& filename, float _lerpTime);
    void setNextAnimation(const std::string& directory, const std::string& filename, float _lerpTime);

    void setMaterial(IConstantBuffer<Material>* material, uint32_t index = 0);
};

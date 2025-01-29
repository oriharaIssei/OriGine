#pragma once

///stl
#include <memory>
#include <functional>
//container
#include <array>
#include <vector>
///engine
#include "directX12/IConstantBuffer.h"
#include "material/Material.h"
#include "transform/Transform.h"

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

    void Init(
        const std::string& _directoryPath,
        const std::string& _filename);
    void Init(const AnimationSetting& _animationSetting);
    void Init(
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

    std::unique_ptr<Model> model_;
    std::string currentAnimationName_;
    std::unique_ptr<Animation> animation_;
public:
    const Model* getModel() const;
    Model* getModel();
    void setModel(std::unique_ptr<Model> model);
    void setModel(const std::string& directory, const std::string& filename);

    const Animation* getAnimation() const;
    Animation* getAnimation();
    void setAnimation(const std::string& directory, const std::string& filename);
    void setAnimation(std::unique_ptr<Animation>& animation);
   
    void setMaterial(IConstantBuffer<Material>* material, uint32_t index = 0);
};

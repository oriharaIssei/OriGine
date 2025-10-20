#pragma once
#include "IComponent.h"

/// stl
#include <memory>
#include <string>

/// ECS
#include "scene/Scene.h"

/// <summary>
/// シーン上で動くシーンのコンポーネント.
/// SceneManagerに管理されない.
/// </summary>
class SubScene
    : public IComponent {
    friend void to_json(nlohmann::json& j, const SubScene& scene);
    friend void from_json(const nlohmann::json& j, SubScene& scene);

public:
    SubScene();
    ~SubScene() override;

    void Initialize(Entity* _entity) override;
    void Edit(Scene* _scene, Entity* _entity, const std::string& _parentLabel) override;
    void Finalize() override;

    /// <summary>
    /// シーンをアクティブにする (必要ならロードする)
    /// </summary>
    void Activate();
    /// <summary>
    /// シーンを非アクティブにする (アンロード)
    /// </summary>
    void Deactivate();

    void Load(const std::string& _sceneName);
    void Unload();

private:
    bool isActive_ = false;

    std::string sceneName_           = "";
    std::shared_ptr<Scene> subScene_ = nullptr;

public:
    const Scene* getSubScene() const { return subScene_.get(); }
    std::shared_ptr<Scene> getSubSceneRef() { return subScene_; }
    const std::string& getSceneName() const { return sceneName_; }

    bool isActive() const { return isActive_; }
    void setActive(bool _active) { isActive_ = _active; }
};

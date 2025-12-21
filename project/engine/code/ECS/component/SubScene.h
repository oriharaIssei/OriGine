#pragma once

#include "IComponent.h"

/// stl
#include <memory>
#include <string>

/// ECS
#include "scene/Scene.h"

namespace OriGine {

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

    void Initialize(Scene* _scene, EntityHandle _entity) override;
    void Edit(Scene* _scene, EntityHandle _entity, const ::std::string& _parentLabel) override;
    void Finalize() override;

    /// <summary>
    /// シーンをアクティブにする (必要ならロードする)
    /// </summary>
    void Activate();
    /// <summary>
    /// シーンを非アクティブにする (アンロード)
    /// </summary>
    void Deactivate();

    void Load(const ::std::string& _sceneName);
    void Unload();

private:
    bool isActive_             = false;
    int32_t renderingPriority_ = 0; // 描画優先度 (値が大きいほど前面に描画される)

    ::std::string sceneName_           = "";
    ::std::shared_ptr<Scene> subScene_ = nullptr;

public:
    const Scene* GetSubScene() const { return subScene_.get(); }
    ::std::shared_ptr<Scene> GetSubSceneRef() { return subScene_; }
    const ::std::string& GetSceneName() const { return sceneName_; }

    int32_t GetRenderingPriority() const { return renderingPriority_; }

    bool IsActive() const { return isActive_; }
    void SetActive(bool _active) { isActive_ = _active; }
};

}; // namespace OriGine

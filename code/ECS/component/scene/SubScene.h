#pragma once

#include "component/IComponent.h"

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

    /// <summary>
    /// 初期化処理
    /// </summary>
    void Initialize(Scene* _scene, const EntityHandle& _entity) override;
    /// <summary>
    /// エディタ用編集UI
    /// </summary>
    void Edit(Scene* _scene, const EntityHandle& _entity, const ::std::string& _parentLabel) override;
    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize() override;

    /// <summary>
    /// シーンをアクティブにする (必要ならロードする)
    /// </summary>
    void Activate();
    /// <summary>
    /// シーンを非アクティブにする (アンロード)
    /// </summary>
    void Deactivate();

    /// <summary>
    /// 指定した名前のシーンをサブシーンとして読み込む
    /// </summary>
    /// <param name="_sceneName">読み込むシーン名</param>
    void Load(const ::std::string& _sceneName);
    /// <summary>
    /// 保持しているサブシーンを解放する
    /// </summary>
    void Unload();

private:
    bool isActive_             = false; // サブシーンが現在アクティブかどうか
    int32_t renderingPriority_ = 0; // 描画優先度 (値が大きいほど前面に描画される)

    ::std::string sceneName_           = ""; // ロード対象のシーン名
    ::std::shared_ptr<Scene> subScene_ = nullptr; // 実体化されたサブシーン

public:
    const Scene* GetSubScene() const { return subScene_.get(); }
    ::std::shared_ptr<Scene> GetSubSceneRef() { return subScene_; }
    const ::std::string& GetSceneName() const { return sceneName_; }

    int32_t GetRenderingPriority() const { return renderingPriority_; }

    bool IsActive() const { return isActive_; }
    void SetActive(bool _active) { isActive_ = _active; }
};

}; // namespace OriGine

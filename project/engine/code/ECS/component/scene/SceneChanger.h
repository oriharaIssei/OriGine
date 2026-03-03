#pragma once
/// parent
#include "component/IComponent.h"

/// stl
#include <string>

/// <summary>
/// シーンを変更するコンポーネント
/// </summary>
class SceneChanger
    : public OriGine::IComponent {
    /// <summary>
    /// JSON 変換用
    /// </summary>
    friend void to_json(nlohmann::json& j, const SceneChanger& r);
    /// <summary>
    /// JSON 復元用
    /// </summary>
    friend void from_json(const nlohmann::json& j, SceneChanger& r);

public:
    SceneChanger();
    virtual ~SceneChanger();

    /// <summary>
    /// 初期化処理
    /// </summary>
    virtual void Initialize(OriGine::Scene* _scene, OriGine::EntityHandle _owner) override;
    /// <summary>
    /// エディタ用編集UI
    /// </summary>
    virtual void Edit(OriGine::Scene* _scene, OriGine::EntityHandle _handle, const std::string& _parentLabel) override;

    /// <summary>
    /// 終了処理
    /// </summary>
    virtual void Finalize() override;

    /// <summary>
    /// SceneTransitionSystem を使用する場合に呼び出す
    /// </summary>
    void ChangeScene() {
        isSceneChanged_ = true;
    }

private:
    bool isSceneChanged_ = false;
    std::string nextSceneName_;

public:
    bool isChanged() const { return isSceneChanged_; }
    void SetNextSceneName(const std::string& _nextSceneName) { nextSceneName_ = _nextSceneName; }
    const std::string& GetNextSceneName() const { return nextSceneName_; }
};

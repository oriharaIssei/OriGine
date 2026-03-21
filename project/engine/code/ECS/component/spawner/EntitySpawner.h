#pragma once

/// engine
// ECS
#include "component/IComponent.h"
// spawn control
#include "component/effect/particle/emitter/Emitter.h"

/// externals
#include <nlohmann/json.hpp>

/// stl
#include <string>

namespace OriGine {

// 前方宣言
class EmitterShapeRenderingSystem;

/// <summary>
/// .ent テンプレートから Entity をスポーンするコンポーネント。
/// スポーンされた Entity の寿命管理は Entity 側のコンポーネントが行う。
/// </summary>
class EntitySpawner
    : public IComponent {
    friend void to_json(nlohmann::json& _j, const EntitySpawner& _comp);
    friend void from_json(const nlohmann::json& _j, EntitySpawner& _comp);

    friend class EntitySpawnerWorkSystem;
#ifdef _DEBUG
    friend class EmitterShapeRenderingSystem;
#endif

public:
    EntitySpawner()           = default;
    ~EntitySpawner() override = default;

    void Initialize(Scene* _scene, EntityHandle _entity) override;
    void Finalize() override;
    void Edit(Scene* _scene, EntityHandle _entity, const std::string& _parentLabel) override;

    // ── 再生制御 ──────────────────────────────────────────────

    /// <summary>
    /// 最初から再生開始
    /// </summary>
    void PlayStart() { emitter_.PlayStart(); }
    /// <summary>
    /// 途中から再生再開
    /// </summary>
    void PlayContinue() { emitter_.PlayContinue(); }
    /// <summary>
    /// 再生停止
    /// </summary>
    void PlayStop() { emitter_.PlayStop(); }

private:
    // スポーン制御
    Emitter emitter_;

    // スポーンする .ent テンプレートの型名（ファイル名から拡張子を除いたもの）
    std::string templateTypeName_;

public:
    bool IsActive() const { return emitter_.isActive_; }
    const std::string& GetTemplateTypeName() const { return templateTypeName_; }
    void SetTemplateTypeName(const std::string& _name) { templateTypeName_ = _name; }
};

void to_json(nlohmann::json& _j, const EntitySpawner& _comp);
void from_json(const nlohmann::json& _j, EntitySpawner& _comp);

} // namespace OriGine

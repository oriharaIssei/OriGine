#pragma once

#include "system/ISystem.h"

///stl
#include <vector>
/// engine
#include "scene/Scene.h"

/// <summary>
/// サブシーンのレンダリングを行うシステム
/// </summary>
class SubSceneRender
: public ISystem{
    public:
    SubSceneRender() : ISystem(SystemCategory::PostRender, 0) {}
    void Initialize() override {}
    void Finalize() override {}

    void Update()override;
    void EndRender();

protected:
    void UpdateEntity(Entity* _entity) override;

private:
    std::vector<Scene*> scenes_;
};

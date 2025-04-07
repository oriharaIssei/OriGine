#pragma once

#include "system/ISystem.h"

class LineRenderSystem
    : public ISystem {
public:
    LineRenderSystem() : ISystem(SystemType::Render) {}
    ~LineRenderSystem() {}

    void Initialize() override;
    void Update() override;
    void Finalize() override;

protected:
    void UpdateEntity(GameEntity* _entity) override;

    void CreatePso();
    void StartRender();

private:
};

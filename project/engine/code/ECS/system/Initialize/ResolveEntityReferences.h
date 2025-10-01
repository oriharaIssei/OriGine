#pragma once
#include "system/ISystem.h"

class ResolveEntityReferences
    : public ISystem {
public:
    ResolveEntityReferences();
    ~ResolveEntityReferences() override = default;
    void Initialize() override;
    void Finalize() override;

private:
    void UpdateEntity(GameEntity* _entity) override;
};

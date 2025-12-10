#pragma once
#include "system/ISystem.h"

namespace OriGine {

/// <summary>
/// EntityReferenceListのEntityファイルを読み込むシステム
/// </summary>
class ResolveEntityReferences
    : public ISystem {
public:
    ResolveEntityReferences();
    ~ResolveEntityReferences() override = default;
    void Initialize() override;
    void Finalize() override;

private:
    void UpdateEntity(Entity* _entity) override;
};

} // namespace OriGine

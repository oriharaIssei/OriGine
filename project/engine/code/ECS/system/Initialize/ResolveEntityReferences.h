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

    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize() override;

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize() override;

private:
    /// <summary>
    /// エンティティの更新（Entity間の参照を解決する）
    /// </summary>
    /// <param name="_handle">エンティティハンドル</param>
    void UpdateEntity(EntityHandle _handle) override;
};

} // namespace OriGine

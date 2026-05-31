#pragma once

#include "text/BitmapFont.h"
#include "ECS/component/text/TextComponent.h"

namespace OriGine {

/// <summary>
/// テキストのレイアウト計算を行うステートレスなヘルパー。
/// 使用するフォントは呼び出し側が FontManager 等から解決して渡す。
/// （フォント所有はせず、TextLayout と Renderer / Font 管理を分離する）
/// </summary>
class TextLayoutSystem {
public:
    TextLayoutSystem()  = default;
    ~TextLayoutSystem() = default;

    /// <summary>
    /// テキストコンポーネントのレイアウトを計算し、グリフクアッド列を生成する。
    /// </summary>
    /// <param name="_font">使用するフォント（必要なグリフがあれば追加生成される）</param>
    /// <param name="_text">対象のテキストコンポーネント</param>
    /// <param name="_result">レイアウト結果の出力先</param>
    /// <param name="_consumeDirty">true なら計算後に TextComponent::dirty をクリアする。
    /// 複数システムが同じテキストを参照する場合、消費しない側は false を指定する。</param>
    void UpdateLayout(BitmapFont& _font, TextComponent& _text, TextLayoutResult& _result, bool _consumeDirty = true);
};

} // namespace OriGine

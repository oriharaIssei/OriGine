#pragma once

#include "text/FontTypes.h"
#include "imgui/imstb_truetype.h"
#include "imgui/imstb_rectpack.h"

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace OriGine {

/// <summary>
/// TrueType（stb_truetype）またはビルトインASCIIフォントからグリフアトラスを生成・管理するクラス。
/// 必要になったグリフをオンデマンドでラスタライズしてアトラスに詰め込み（stb_rectpack）、
/// UV等のメトリクス情報を提供する。
/// </summary>
class BitmapFont {
public:
    BitmapFont() = default;
    ~BitmapFont() = default;

    /// <summary>
    /// TrueTypeを読み込まずに使う、5x7ドットのビルトインASCIIフォントでアトラスを生成する。
    /// </summary>
    /// <param name="_fontSize">セルサイズの目安（px）</param>
    void GenerateAsciiAtlas(int _fontSize = 16);

    /// <summary>
    /// TrueType/OpenTypeフォントファイルを読み込み、主フォントとして設定する。
    /// 半角ASCII（32〜126）とフォールバック用グリフ(U+FFFD)を先読みでラスタライズする。
    /// </summary>
    /// <param name="_path">フォントファイルのパス（.ttf / .ttc）</param>
    /// <param name="_fontSize">ベースとなるピクセルサイズ</param>
    /// <returns>読み込みに成功したら true</returns>
    bool LoadFromFile(const std::string& _path, float _fontSize);

    /// <summary>
    /// フォールバックフォントを追加する。主フォントに存在しないグリフは、
    /// 追加順にフォールバックフォントから探索して同一アトラスにラスタライズする。
    /// 主フォントが TrueType として読み込まれている必要がある。
    /// </summary>
    /// <param name="_path">フォールバックフォントファイルのパス（.ttf / .ttc）</param>
    /// <returns>読み込みに成功したら true</returns>
    bool AddFallback(const std::string& _path);

    /// <summary>
    /// 指定コードポイントのグリフがアトラスに存在することを保証する。
    /// 未登録ならラスタライズを試み、アトラスが満杯なら拡張して再試行する。
    /// </summary>
    /// <param name="_codepoint">Unicodeコードポイント</param>
    /// <returns>グリフが利用可能になったら true</returns>
    bool EnsureGlyph(uint32_t _codepoint);

    /// <summary>
    /// UTF-8文字列に含まれる全文字についてグリフの存在を保証する。
    /// </summary>
    /// <param name="_utf8Text">UTF-8文字列</param>
    /// <param name="_len">バイト長</param>
    /// <returns>1つでも新規にラスタライズしたら true</returns>
    bool EnsureGlyphs(const char* _utf8Text, size_t _len);

    const GlyphMetrics& GetGlyph(uint32_t _codepoint) const;
    bool HasGlyph(uint32_t _codepoint) const;
    int GetAtlasWidth() const { return atlasWidth_; }
    int GetAtlasHeight() const { return atlasHeight_; }
    const std::vector<uint8_t>& GetAtlasPixels() const { return atlasPixels_; }
    float GetLineHeight() const { return lineHeight_; }

    float GetFontSize() const { return fontSize_; }
    bool IsAtlasDirty() const { return atlasDirty_; }
    void ClearAtlasDirty() { atlasDirty_ = false; }
    bool IsTrueType() const { return ttfLoaded_; }

private:
    /// <summary>
    /// アトラスサイズを2倍に拡張し、既存グリフを再パック・再ラスタライズする。
    /// </summary>
    /// <returns>kMaxAtlasSizeを超える場合は false</returns>
    bool GrowAtlas();

    /// <summary>
    /// 指定コードポイントのグリフを面（主/フォールバック）から取得し、
    /// stb_rectpackでアトラス内の空き領域に配置してラスタライズする。
    /// </summary>
    bool PackAndRasterizeGlyph(uint32_t _codepoint);

    /// <summary>
    /// stb_rectpackのパッキング状態を現在のアトラスサイズで初期化する。
    /// </summary>
    void ResetPacker();

    /// <summary>
    /// 指定コードポイントを持つフォント面（主→フォールバックの順）を選択する。
    /// どの面にも存在しない場合は false を返し、主フォントの glyphIndex=0 を返す。
    /// </summary>
    bool SelectFaceForGlyph(uint32_t _codepoint,
                            const stbtt_fontinfo*& _outInfo,
                            float& _outScale,
                            int& _outGlyphIndex) const;

    /// <summary>
    /// フォールバックフォント 1 つ分の情報。
    /// </summary>
    struct FontFace {
        std::vector<uint8_t> fileData;
        stbtt_fontinfo info{};
        float scale = 0.0f;
        int ascent  = 0;
        int descent = 0;
        int lineGap = 0;
    };
    std::vector<FontFace> fallbacks_;

    std::unordered_map<uint32_t, GlyphMetrics> glyphMap_;
    GlyphMetrics fallbackGlyph_{};
    std::vector<uint8_t> atlasPixels_;
    int atlasWidth_ = 0;
    int atlasHeight_ = 0;
    float lineHeight_ = 0.0f;
    bool atlasDirty_ = false;

    std::vector<uint8_t> fontFileData_;
    stbtt_fontinfo fontInfo_{};
    float fontScale_ = 0.0f;
    float fontSize_ = 0.0f;
    bool ttfLoaded_ = false;
    int ascent_ = 0;
    int descent_ = 0;
    int lineGap_ = 0;

    static constexpr int kInitialAtlasSize = 512;
    static constexpr int kMaxAtlasSize = 4096;
    static constexpr int kGlyphPadding = 1;

    std::vector<stbrp_node> packNodes_;
    stbrp_context packContext_{};
    bool packerReady_ = false;

    static constexpr int kFirstChar = 32;
    static constexpr int kLastChar = 126;
    static constexpr int kGlyphCount = kLastChar - kFirstChar + 1;
    std::array<GlyphMetrics, 128> builtinGlyphs_{};
    bool usingBuiltin_ = false;
};

} // namespace OriGine

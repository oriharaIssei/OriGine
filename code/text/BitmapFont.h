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

class BitmapFont {
public:
    BitmapFont() = default;
    ~BitmapFont() = default;

    void GenerateAsciiAtlas(int _fontSize = 16);

    bool LoadFromFile(const std::string& _path, float _fontSize);

    /// <summary>
    /// フォールバックフォントを追加する。主フォントに存在しないグリフは、
    /// 追加順にフォールバックフォントから探索して同一アトラスにラスタライズする。
    /// 主フォントが TrueType として読み込まれている必要がある。
    /// </summary>
    /// <param name="_path">フォールバックフォントファイルのパス（.ttf / .ttc）</param>
    /// <returns>読み込みに成功したら true</returns>
    bool AddFallback(const std::string& _path);

    bool EnsureGlyph(uint32_t _codepoint);
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
    bool GrowAtlas();
    bool PackAndRasterizeGlyph(uint32_t _codepoint);
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

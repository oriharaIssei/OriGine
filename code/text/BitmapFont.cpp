#include "text/BitmapFont.h"

#include <algorithm>
#include <cstring>
#include <fstream>

namespace OriGine {

namespace {

constexpr int kBuiltinCellW = 8;
constexpr int kBuiltinCellH = 16;
constexpr int kCols = 16;

void RasterizeCharToBuffer(uint8_t* _dst, int _dstPitch, char _ch, int _cellW, int _cellH) {
    for (int y = 0; y < _cellH; ++y) {
        std::memset(_dst + y * _dstPitch, 0, _cellW);
    }

    if (_ch < 33 || _ch > 126) return;

    static const uint8_t kFont5x7[][7] = {
        {0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04},
        {0x0A, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0x0A, 0x1F, 0x0A, 0x0A, 0x1F, 0x0A, 0x00},
        {0x04, 0x0F, 0x14, 0x0E, 0x05, 0x1E, 0x04},
        {0x18, 0x19, 0x02, 0x04, 0x08, 0x13, 0x03},
        {0x08, 0x14, 0x14, 0x08, 0x15, 0x12, 0x0D},
        {0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02},
        {0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08},
        {0x00, 0x04, 0x15, 0x0E, 0x15, 0x04, 0x00},
        {0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00},
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x08},
        {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04},
        {0x01, 0x02, 0x02, 0x04, 0x08, 0x08, 0x10},
        {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E},
        {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E},
        {0x0E, 0x11, 0x01, 0x06, 0x08, 0x10, 0x1F},
        {0x0E, 0x11, 0x01, 0x06, 0x01, 0x11, 0x0E},
        {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02},
        {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E},
        {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E},
        {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08},
        {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E},
        {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C},
        {0x00, 0x00, 0x04, 0x00, 0x00, 0x04, 0x00},
        {0x00, 0x00, 0x04, 0x00, 0x00, 0x04, 0x08},
        {0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02},
        {0x00, 0x00, 0x1F, 0x00, 0x1F, 0x00, 0x00},
        {0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08},
        {0x0E, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04},
        {0x0E, 0x11, 0x17, 0x15, 0x17, 0x10, 0x0E},
        {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11},
        {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E},
        {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E},
        {0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E},
        {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F},
        {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10},
        {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0E},
        {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11},
        {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E},
        {0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C},
        {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11},
        {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F},
        {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11},
        {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11},
        {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E},
        {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10},
        {0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D},
        {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11},
        {0x0E, 0x11, 0x10, 0x0E, 0x01, 0x11, 0x0E},
        {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04},
        {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E},
        {0x11, 0x11, 0x11, 0x11, 0x0A, 0x0A, 0x04},
        {0x11, 0x11, 0x11, 0x15, 0x15, 0x1B, 0x11},
        {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11},
        {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04},
        {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F},
        {0x0E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0E},
        {0x10, 0x08, 0x08, 0x04, 0x02, 0x02, 0x01},
        {0x0E, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0E},
        {0x04, 0x0A, 0x11, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F},
        {0x08, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x0E, 0x01, 0x0F, 0x11, 0x0F},
        {0x10, 0x10, 0x1E, 0x11, 0x11, 0x11, 0x1E},
        {0x00, 0x00, 0x0E, 0x11, 0x10, 0x11, 0x0E},
        {0x01, 0x01, 0x0F, 0x11, 0x11, 0x11, 0x0F},
        {0x00, 0x00, 0x0E, 0x11, 0x1F, 0x10, 0x0E},
        {0x06, 0x08, 0x1E, 0x08, 0x08, 0x08, 0x08},
        {0x00, 0x00, 0x0F, 0x11, 0x0F, 0x01, 0x0E},
        {0x10, 0x10, 0x1E, 0x11, 0x11, 0x11, 0x11},
        {0x04, 0x00, 0x0C, 0x04, 0x04, 0x04, 0x0E},
        {0x02, 0x00, 0x06, 0x02, 0x02, 0x12, 0x0C},
        {0x10, 0x10, 0x12, 0x14, 0x18, 0x14, 0x12},
        {0x0C, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E},
        {0x00, 0x00, 0x1A, 0x15, 0x15, 0x15, 0x15},
        {0x00, 0x00, 0x1E, 0x11, 0x11, 0x11, 0x11},
        {0x00, 0x00, 0x0E, 0x11, 0x11, 0x11, 0x0E},
        {0x00, 0x00, 0x1E, 0x11, 0x1E, 0x10, 0x10},
        {0x00, 0x00, 0x0F, 0x11, 0x0F, 0x01, 0x01},
        {0x00, 0x00, 0x16, 0x19, 0x10, 0x10, 0x10},
        {0x00, 0x00, 0x0F, 0x10, 0x0E, 0x01, 0x1E},
        {0x08, 0x08, 0x1E, 0x08, 0x08, 0x09, 0x06},
        {0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 0x0F},
        {0x00, 0x00, 0x11, 0x11, 0x11, 0x0A, 0x04},
        {0x00, 0x00, 0x11, 0x11, 0x15, 0x15, 0x0A},
        {0x00, 0x00, 0x11, 0x0A, 0x04, 0x0A, 0x11},
        {0x00, 0x00, 0x11, 0x11, 0x0F, 0x01, 0x0E},
        {0x00, 0x00, 0x1F, 0x02, 0x04, 0x08, 0x1F},
        {0x02, 0x04, 0x04, 0x08, 0x04, 0x04, 0x02},
        {0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04},
        {0x08, 0x04, 0x04, 0x02, 0x04, 0x04, 0x08},
        {0x00, 0x00, 0x08, 0x15, 0x02, 0x00, 0x00},
    };

    int idx = static_cast<int>(_ch) - 33;
    if (idx < 0 || idx >= static_cast<int>(sizeof(kFont5x7) / sizeof(kFont5x7[0]))) return;

    const uint8_t* pattern = kFont5x7[idx];
    int offsetX = (_cellW - 5) / 2;
    int offsetY = (_cellH - 7) / 2;

    for (int row = 0; row < 7; ++row) {
        uint8_t bits = pattern[row];
        for (int col = 0; col < 5; ++col) {
            if (bits & (0x10 >> col)) {
                int px = offsetX + col;
                int py = offsetY + row;
                if (px >= 0 && px < _cellW && py >= 0 && py < _cellH) {
                    _dst[py * _dstPitch + px] = 255;
                }
            }
        }
    }
}

uint32_t DecodeUtf8Char(const char* _str, size_t _len, size_t& _outBytes) {
    if (_len == 0) { _outBytes = 0; return 0; }
    uint8_t c = static_cast<uint8_t>(_str[0]);
    if (c < 0x80) {
        _outBytes = 1;
        return c;
    } else if ((c & 0xE0) == 0xC0 && _len >= 2) {
        _outBytes = 2;
        return ((c & 0x1F) << 6) | (static_cast<uint8_t>(_str[1]) & 0x3F);
    } else if ((c & 0xF0) == 0xE0 && _len >= 3) {
        _outBytes = 3;
        return ((c & 0x0F) << 12) |
               ((static_cast<uint8_t>(_str[1]) & 0x3F) << 6) |
               (static_cast<uint8_t>(_str[2]) & 0x3F);
    } else if ((c & 0xF8) == 0xF0 && _len >= 4) {
        _outBytes = 4;
        return ((c & 0x07) << 18) |
               ((static_cast<uint8_t>(_str[1]) & 0x3F) << 12) |
               ((static_cast<uint8_t>(_str[2]) & 0x3F) << 6) |
               (static_cast<uint8_t>(_str[3]) & 0x3F);
    }
    _outBytes = 1;
    return 0xFFFD;
}

} // namespace

void BitmapFont::GenerateAsciiAtlas(int _fontSize) {
    usingBuiltin_ = true;
    ttfLoaded_ = false;
    fontSize_ = static_cast<float>(_fontSize);

    int cellW = (std::max)(kBuiltinCellW, _fontSize);
    int cellH = (std::max)(kBuiltinCellH, _fontSize);
    int cols = kCols;
    int totalGlyphs = kGlyphCount + 1;
    int rows = (totalGlyphs + cols - 1) / cols;

    atlasWidth_ = cols * cellW;
    atlasHeight_ = rows * cellH;
    lineHeight_ = static_cast<float>(cellH);

    atlasPixels_.resize(atlasWidth_ * atlasHeight_, 0);

    float invW = 1.0f / static_cast<float>(atlasWidth_);
    float invH = 1.0f / static_cast<float>(atlasHeight_);

    for (int i = 0; i < kGlyphCount; ++i) {
        char ch = static_cast<char>(kFirstChar + i);
        int col = i % cols;
        int row = i / cols;
        int px = col * cellW;
        int py = row * cellH;

        RasterizeCharToBuffer(atlasPixels_.data() + py * atlasWidth_ + px,
                              atlasWidth_, ch, cellW, cellH);

        GlyphMetrics& g = builtinGlyphs_[static_cast<int>(ch)];
        g.uvMin = {px * invW, py * invH};
        g.uvMax = {(px + cellW) * invW, (py + cellH) * invH};
        g.size = {static_cast<float>(cellW), static_cast<float>(cellH)};
        g.bearing = {0.0f, 0.0f};
        g.advance = static_cast<float>(cellW);
    }

    GlyphMetrics& sp = builtinGlyphs_[32];
    sp.size = {static_cast<float>(cellW), static_cast<float>(cellH)};
    sp.advance = static_cast<float>(cellW);

    {
        int fbIdx = kGlyphCount;
        int col = fbIdx % cols;
        int row = fbIdx / cols;
        int px = col * cellW;
        int py = row * cellH;

        int ox = (cellW - 5) / 2;
        int oy = (cellH - 7) / 2;
        uint8_t* dst = atlasPixels_.data() + py * atlasWidth_ + px;
        for (int c = 0; c < 5; ++c) {
            dst[oy * atlasWidth_ + ox + c] = 255;
            dst[(oy + 6) * atlasWidth_ + ox + c] = 255;
        }
        for (int r = 0; r < 7; ++r) {
            dst[(oy + r) * atlasWidth_ + ox] = 255;
            dst[(oy + r) * atlasWidth_ + ox + 4] = 255;
        }

        fallbackGlyph_.uvMin = {px * invW, py * invH};
        fallbackGlyph_.uvMax = {(px + cellW) * invW, (py + cellH) * invH};
        fallbackGlyph_.size = {static_cast<float>(cellW), static_cast<float>(cellH)};
        fallbackGlyph_.bearing = {0.0f, 0.0f};
        fallbackGlyph_.advance = static_cast<float>(cellW);
    }

    atlasDirty_ = true;
}

bool BitmapFont::LoadFromFile(const std::string& _path, float _fontSize) {
    std::ifstream file(_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return false;

    auto fileSize = file.tellg();
    if (fileSize <= 0) return false;
    file.seekg(0);

    fontFileData_.resize(static_cast<size_t>(fileSize));
    file.read(reinterpret_cast<char*>(fontFileData_.data()), fileSize);
    if (!file) return false;

    if (!stbtt_InitFont(&fontInfo_, fontFileData_.data(),
                        stbtt_GetFontOffsetForIndex(fontFileData_.data(), 0))) {
        fontFileData_.clear();
        return false;
    }

    fontSize_ = _fontSize;
    fontScale_ = stbtt_ScaleForPixelHeight(&fontInfo_, _fontSize);
    stbtt_GetFontVMetrics(&fontInfo_, &ascent_, &descent_, &lineGap_);
    lineHeight_ = (ascent_ - descent_ + lineGap_) * fontScale_;

    ttfLoaded_ = true;
    usingBuiltin_ = false;

    atlasWidth_ = kInitialAtlasSize;
    atlasHeight_ = kInitialAtlasSize;
    atlasPixels_.assign(atlasWidth_ * atlasHeight_, 0);
    glyphMap_.clear();

    ResetPacker();

    for (uint32_t cp = 32; cp <= 126; ++cp) {
        PackAndRasterizeGlyph(cp);
    }

    fallbackGlyph_ = {};
    PackAndRasterizeGlyph(0xFFFD);
    if (glyphMap_.count(0xFFFD)) {
        fallbackGlyph_ = glyphMap_[0xFFFD];
    } else {
        fallbackGlyph_.advance = fontSize_ * 0.5f;
        fallbackGlyph_.size = {fontSize_ * 0.5f, fontSize_};
    }

    atlasDirty_ = true;
    return true;
}

bool BitmapFont::AddFallback(const std::string& _path) {
    // 主フォントが TrueType でなければフォールバックは機能しない
    if (!ttfLoaded_) return false;

    std::ifstream file(_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return false;

    auto fileSize = file.tellg();
    if (fileSize <= 0) return false;
    file.seekg(0);

    FontFace face;
    face.fileData.resize(static_cast<size_t>(fileSize));
    file.read(reinterpret_cast<char*>(face.fileData.data()), fileSize);
    if (!file) return false;

    if (!stbtt_InitFont(&face.info, face.fileData.data(),
                        stbtt_GetFontOffsetForIndex(face.fileData.data(), 0))) {
        return false;
    }

    face.scale = stbtt_ScaleForPixelHeight(&face.info, fontSize_);
    stbtt_GetFontVMetrics(&face.info, &face.ascent, &face.descent, &face.lineGap);

    // std::vector<uint8_t> は move でバッファアドレスが保たれるため info.data は有効なまま
    fallbacks_.push_back(std::move(face));
    return true;
}

bool BitmapFont::SelectFaceForGlyph(uint32_t _codepoint,
                                    const stbtt_fontinfo*& _outInfo,
                                    float& _outScale,
                                    int& _outGlyphIndex) const {
    int gi = stbtt_FindGlyphIndex(&fontInfo_, _codepoint);
    if (gi != 0) {
        _outInfo       = &fontInfo_;
        _outScale      = fontScale_;
        _outGlyphIndex = gi;
        return true;
    }

    for (const auto& fb : fallbacks_) {
        int g = stbtt_FindGlyphIndex(&fb.info, _codepoint);
        if (g != 0) {
            _outInfo       = &fb.info;
            _outScale      = fb.scale;
            _outGlyphIndex = g;
            return true;
        }
    }

    // どの面にも存在しない
    _outInfo       = &fontInfo_;
    _outScale      = fontScale_;
    _outGlyphIndex = 0;
    return false;
}

void BitmapFont::ResetPacker() {
    packNodes_.resize(atlasWidth_);
    stbrp_init_target(&packContext_, atlasWidth_, atlasHeight_,
                      packNodes_.data(), static_cast<int>(packNodes_.size()));
    packerReady_ = true;
}

bool BitmapFont::PackAndRasterizeGlyph(uint32_t _codepoint) {
    if (!ttfLoaded_ || !packerReady_) return false;

    // 主フォント→フォールバックの順でグリフを持つ面を選択
    const stbtt_fontinfo* info = &fontInfo_;
    float scale                = fontScale_;
    int glyphIndex             = 0;
    bool found                 = SelectFaceForGlyph(_codepoint, info, scale, glyphIndex);
    if (!found && _codepoint != 0) return false;

    int x0, y0, x1, y1;
    stbtt_GetGlyphBitmapBox(info, glyphIndex, scale, scale,
                            &x0, &y0, &x1, &y1);

    int gw = x1 - x0;
    int gh = y1 - y0;

    if (gw <= 0 || gh <= 0) {
        int advW, lsb;
        stbtt_GetGlyphHMetrics(info, glyphIndex, &advW, &lsb);

        GlyphMetrics m{};
        m.advance = advW * scale;
        m.size = {0.0f, 0.0f};
        glyphMap_[_codepoint] = m;
        return true;
    }

    int paddedW = gw + kGlyphPadding * 2;
    int paddedH = gh + kGlyphPadding * 2;

    stbrp_rect rect{};
    rect.w = static_cast<stbrp_coord>(paddedW);
    rect.h = static_cast<stbrp_coord>(paddedH);

    if (!stbrp_pack_rects(&packContext_, &rect, 1) || !rect.was_packed) {
        return false;
    }

    int destX = rect.x + kGlyphPadding;
    int destY = rect.y + kGlyphPadding;

    stbtt_MakeGlyphBitmap(info,
                          atlasPixels_.data() + destY * atlasWidth_ + destX,
                          gw, gh, atlasWidth_,
                          scale, scale, glyphIndex);

    float invW = 1.0f / static_cast<float>(atlasWidth_);
    float invH = 1.0f / static_cast<float>(atlasHeight_);

    int advW, lsb;
    stbtt_GetGlyphHMetrics(info, glyphIndex, &advW, &lsb);

    // ベースラインは主フォント基準で揃える（フォールバックグリフも同じベースライン上に置く）
    float scaledAscent = ascent_ * fontScale_;

    GlyphMetrics m{};
    m.uvMin = {destX * invW, destY * invH};
    m.uvMax = {(destX + gw) * invW, (destY + gh) * invH};
    m.size = {static_cast<float>(gw), static_cast<float>(gh)};
    m.bearing = {static_cast<float>(x0), scaledAscent + static_cast<float>(y0)};
    m.advance = advW * scale;
    glyphMap_[_codepoint] = m;

    return true;
}

bool BitmapFont::GrowAtlas() {
    int newSize = atlasWidth_ * 2;
    if (newSize > kMaxAtlasSize) return false;

    std::vector<uint8_t> newPixels(newSize * newSize, 0);
    for (int y = 0; y < atlasHeight_; ++y) {
        std::memcpy(newPixels.data() + y * newSize,
                    atlasPixels_.data() + y * atlasWidth_,
                    atlasWidth_);
    }

    atlasWidth_ = newSize;
    atlasHeight_ = newSize;
    atlasPixels_ = std::move(newPixels);

    ResetPacker();

    std::unordered_map<uint32_t, GlyphMetrics> oldGlyphs = std::move(glyphMap_);
    glyphMap_.clear();

    for (auto& [cp, _] : oldGlyphs) {
        PackAndRasterizeGlyph(cp);
    }

    atlasDirty_ = true;
    return true;
}

bool BitmapFont::EnsureGlyph(uint32_t _codepoint) {
    if (!ttfLoaded_) return false;
    if (glyphMap_.count(_codepoint)) return true;

    if (PackAndRasterizeGlyph(_codepoint)) {
        atlasDirty_ = true;
        return true;
    }

    if (GrowAtlas()) {
        if (PackAndRasterizeGlyph(_codepoint)) {
            atlasDirty_ = true;
            return true;
        }
    }

    return false;
}

bool BitmapFont::EnsureGlyphs(const char* _utf8Text, size_t _len) {
    bool anyNew = false;
    size_t i = 0;
    while (i < _len) {
        size_t bytes = 0;
        uint32_t cp = DecodeUtf8Char(_utf8Text + i, _len - i, bytes);
        i += bytes;
        if (cp == '\n' || cp == '\r') continue;

        if (!glyphMap_.count(cp)) {
            if (EnsureGlyph(cp)) {
                anyNew = true;
            }
        }
    }
    return anyNew;
}

const GlyphMetrics& BitmapFont::GetGlyph(uint32_t _codepoint) const {
    if (usingBuiltin_) {
        if (_codepoint >= kFirstChar && _codepoint <= kLastChar) {
            return builtinGlyphs_[_codepoint];
        }
        return fallbackGlyph_;
    }

    auto it = glyphMap_.find(_codepoint);
    if (it != glyphMap_.end()) {
        return it->second;
    }
    return fallbackGlyph_;
}

bool BitmapFont::HasGlyph(uint32_t _codepoint) const {
    if (usingBuiltin_) {
        return _codepoint >= kFirstChar && _codepoint <= kLastChar;
    }
    return glyphMap_.count(_codepoint) > 0;
}

} // namespace OriGine

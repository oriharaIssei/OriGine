#pragma once

#include <cstdint>
#include <Vector2.h>
#include <Vector4.h>

namespace OriGine {

struct GlyphMetrics {
    Vec2f uvMin;
    Vec2f uvMax;
    Vec2f size;
    Vec2f bearing;
    float advance = 0.0f;
};

struct GlyphQuad {
    Vec2f posMin;
    Vec2f posMax;
    Vec2f uvMin;
    Vec2f uvMax;
    Vec4f color;
};

struct TextVertex {
    Vec4f pos;
    Vec2f uv;
    Vec4f color;
};

using FontHandle = uint32_t;
constexpr FontHandle kInvalidFontHandle = 0xFFFFFFFF;

} // namespace OriGine

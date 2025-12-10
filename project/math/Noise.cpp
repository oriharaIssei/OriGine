#include "Noise.h"

namespace OriGine {
namespace FractalBrownianMotionNoise {

float Fract(float _x) {
    return _x - std::floor(_x);
}

Vec2f Floor(const Vec2f& _v) {
    return Vec2f(std::floor(_v[X]), std::floor(_v[Y]));
}

Vec2f Fract(const Vec2f& _v) {
    return Vec2f(Fract(_v[X]), Fract(_v[Y]));
}

float Random(const Vec2f& _st) {
    return Fract(std::sin(_st[X] * 12.9898f + _st[Y] * 78.233f) * 43758.5453123f);
}

float Noise(const Vec2f& _st) {
    Vec2f i = Floor(_st);
    Vec2f f = Fract(_st);

    float a = Random(i);
    float b = Random(Vec2f(i[X] + 1.0f, i[Y]));
    float c = Random(Vec2f(i[X], i[Y] + 1.0f));
    float d = Random(Vec2f(i[X] + 1.0f, i[Y] + 1.0f));

    Vec2f u(f[X] * f[X] * (3.0f - 2.0f * f[X]),
        f[Y] * f[Y] * (3.0f - 2.0f * f[Y]));

    return std::lerp(a, b, u[X])
           + (c - a) * u[Y] * (1.0f - u[X])
           + (d - b) * u[X] * u[Y];
}

float Fbm(Vec2f _st) {
    constexpr int kOctaves = 6;
    float value            = 0.0f;
    float amplitude        = 0.5f;

    for (int i = 0; i < kOctaves; i++) {
        value += amplitude * Noise(_st);
        _st[X] *= 2.0f;
        _st[Y] *= 2.0f;
        amplitude *= 0.5f;
    }
    return value;
}

Vec4f ShadePixel(const Vec2f& _fragCoord, const Vec2f& _resolution) {
    Vec2f st(_fragCoord[X] / _resolution[X],
        _fragCoord[Y] / _resolution[Y]);

    // アスペクト比補正
    st[X] *= _resolution[X] / _resolution[Y];

    Vec3f color(0, 0, 0);
    float f = Fbm(Vec2f(st[X] * 3.0f, st[Y] * 3.0f));
    color   = Vec3f(f, f, f);

    return Vec4f(color[X], color[Y], color[Z], 1.0f);
}

} // namespace FractalBrownianMotionNoise
} // namespace OriGine

#include "Noise.h"

namespace OriGine {
namespace FractalBrownianMotionNoise {

float Fract(float x) {
    return x - std::floor(x);
}

Vec2f Floor(const Vec2f& v) {
    return Vec2f(std::floor(v[X]), std::floor(v[Y]));
}

Vec2f Fract(const Vec2f& v) {
    return Vec2f(Fract(v[X]), Fract(v[Y]));
}

/// <summary>
/// 座標から0〜1の擬似乱数を得る(ハッシュ関数)。
/// 同じ座標には必ず同じ値を返すため、乱数表を持たずにノイズを再現できる。
/// </summary>
/// <remarks>
/// GLSLのシェーダで広く使われる定番のハッシュ。乱数性のある数学的な導出ではなく、
/// 「sinの結果を巨大な数で拡大し、小数部だけを取り出す」ことで値を撹拌している。
/// 12.9898 / 78.233 / 43758.5453123 は経験的に良い散らばりが得られるとされる定数で、
/// 意味のある数値ではないため変更しないこと(規則性が見えるノイズになる)。
/// </remarks>
float Random(const Vec2f& st) {
    return Fract(std::sin(st[X] * 12.9898f + st[Y] * 78.233f) * 43758.5453123f);
}

/// <summary>
/// 2次元のバリューノイズ。格子点ごとの乱数を滑らかに補間した、連続的なノイズ値を返す。
/// </summary>
/// <remarks>
/// Random()をそのまま使うと隣り合う座標で値が飛び、砂嵐にしかならない。
/// 座標を整数部(格子の位置)と小数部(格子内の位置)に分け、
/// 囲む4つの格子点の乱数を格子内位置で補間することで滑らかに繋げる。
/// </remarks>
float Noise(const Vec2f& st) {
    // i = どの格子か, f = その格子の中でどこか(0〜1)
    Vec2f i = Floor(st);
    Vec2f f = Fract(st);

    // 現在地を囲む4つの格子点(左下・右下・左上・右上)の乱数値
    float a = Random(i);
    float b = Random(Vec2f(i[X] + 1.0f, i[Y]));
    float c = Random(Vec2f(i[X], i[Y] + 1.0f));
    float d = Random(Vec2f(i[X] + 1.0f, i[Y] + 1.0f));

    // スムーズステップ 3t^2 - 2t^3 で補間係数をならす。
    // fをそのまま線形補間に使うと格子の境界で傾きが不連続になり、格子模様が目に見えてしまう。
    // この式は t=0,1 で微分値が0になるため、境界が滑らかに繋がる
    Vec2f u(f[X] * f[X] * (3.0f - 2.0f * f[X]),
        f[Y] * f[Y] * (3.0f - 2.0f * f[Y]));

    // 4点の双線形補間を展開した形。
    // lerp(lerp(a,b,ux), lerp(c,d,ux), uy) と同値だが、乗算回数を減らした定番の書き方
    return std::lerp(a, b, u[X])
           + (c - a) * u[Y] * (1.0f - u[X])
           + (d - b) * u[X] * u[Y];
}

/// <summary>
/// フラクタルブラウン運動ノイズ。周波数の異なるノイズを重ね合わせ、
/// 雲や地形のような「粗い形の上に細かいディテールが乗る」自然な模様を作る。
/// </summary>
/// <remarks>
/// Noise()を1回呼ぶだけでは滑らかすぎて単調になる。
/// 1オクターブごとに座標を2倍(細かく)・振幅を1/2倍(弱く)しながら加算することで、
/// 大きなうねりが支配的で細部ほど影響が小さい、自然物に近い分布が得られる。
/// 振幅の初期値0.5と1/2倍の等比数列により、合計は 0.5+0.25+... < 1 に収まり、
/// オクターブ数を変えても結果がおおよそ0〜1に留まる。
/// kOctavesを増やすほど細部が出るが、その分Noise()の呼び出しが線形に増える。
/// </remarks>
float Fbm(Vec2f st) {
    constexpr int kOctaves = 6;
    float value            = 0.0f;
    float amplitude        = 0.5f;

    for (int i = 0; i < kOctaves; i++) {
        value += amplitude * Noise(st);
        // 座標を2倍にする＝ノイズの周波数を2倍にする(同じ範囲により細かい模様が入る)
        st[X] *= 2.0f;
        st[Y] *= 2.0f;
        amplitude *= 0.5f;
    }
    return value;
}

Vec4f ShadePixel(const Vec2f& fragCoord, const Vec2f& resolution) {
    Vec2f st(fragCoord[X] / resolution[X],
        fragCoord[Y] / resolution[Y]);

    // アスペクト比補正
    // stは0〜1に正規化された座標なので、そのまま使うと画面が横長のときノイズも横に引き伸ばされる。
    // X側にアスペクト比を掛け直すことで、模様が縦横同じ密度の等方的な見た目になる
    st[X] *= resolution[X] / resolution[Y];

    Vec3f color(0, 0, 0);
    float f = Fbm(Vec2f(st[X] * 3.0f, st[Y] * 3.0f));
    color   = Vec3f(f, f, f);

    return Vec4f(color[X], color[Y], color[Z], 1.0f);
}

} // namespace FractalBrownianMotionNoise
} // namespace OriGine

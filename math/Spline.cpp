#include "Spline.h"
#include <limits>

using namespace OriGine;

/// <summary>
/// Catmull-Romスプラインで、p1とp2の間を補間した座標を返す。
/// </summary>
/// <remarks>
/// ベジェ曲線と違い制御点を「通る」曲線なので、レールやパスのように
/// 指定した点の上を必ず通ってほしい用途に向く。
/// 補間されるのは中央のp1〜p2の区間だけで、両端のp0とp3は
/// その区間の接線(曲がり具合)を決めるためだけに使う。
/// </remarks>
/// <param name="_t">p1(0.0)からp2(1.0)までの位置を表す比率</param>
Vec3f OriGine::CatmullRomSpline(const Vec3f& p0, const Vec3f& p1, const Vec3f& p2, const Vec3f& p3, float t) {
    float t2 = t * t;
    float t3 = t2 * t;

    // 標準的なCatmull-Romの多項式を展開した形。
    // 各制御点に掛かる係数はtの3次多項式として事前に整理されており、
    // 全体の1/2倍は接線を(p2-p0)/2として定義することに由来する。
    // t=0でp1、t=1でp2に一致するよう係数が組まれているため、値を個別に変えないこと
    return 0.5f * ((2.0f * p1) + (-p0 + p2) * t + (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 + (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);
}

std::deque<Vec3f> OriGine::CatmullRomSpline(const std::deque<Vec3f>& points, int samplePerSegment) {
    std::deque<Vec3f> result;

    /// コントロールポイントが4個以下なら何もできない
    /// 1区間の補間に前後の点を含めた4点が必要なため。
    /// なおsize_tは符号なしなので、この判定が無いと下のsize()-3が巨大な値に回り込み、
    /// ループが暴走して範囲外参照になる
    if (points.size() < 4) {
        return result;
    }

    // 4点の窓を1つずつずらしながら、各窓の中央区間(points[i+1]〜points[i+2])を補間する。
    // そのため生成される曲線は先頭と末尾の点を通らず、内側の点だけを繋いだものになる
    for (size_t i = 0; i < points.size() - 3; i++) {
        for (size_t s = 0; s < samplePerSegment; s++) {
            float t = static_cast<float>(s) / samplePerSegment;
            result.push_back(CatmullRomSpline(
                points[i + 0],
                points[i + 1],
                points[i + 2],
                points[i + 3],
                t));
        }
    }

    // 最後のポイントも追加（p2）
    // 内側ループのtは s/samplePerSegment なので 1.0 に到達せず、各区間の終点が欠ける。
    // 最終区間の終点だけは繋がりが途切れて見えるため、ここで明示的に足している
    result.push_back(points[points.size() - 2]);
    return result;
}

float OriGine::CalcSplineLength(const std::deque<Vec3f>& points) {
    if (points.empty()) {
        return 0.f;
    }
    float totalLength = 0.0f;
    for (size_t i = 0; i < points.size() - 1; i++) {
        Vec3f p1 = points[i];
        Vec3f p2 = points[i + 1];
        totalLength += Vec3f(p2 - p1).length();
    }
    return totalLength;
}

std::pair<uint32_t, uint32_t> OriGine::CalcPointSegmentIndex(const std::deque<Vec3f>& points, const Vec3f& position) {
    if (points.empty()) {
        return {0, 0};
    }
    uint32_t closestIndex1 = 0;
    uint32_t closestIndex2 = 1;
    float minDistanceSq    = (std::numeric_limits<float>::max)();
    for (size_t i = 0; i < points.size() - 1; i++) {
        Vec3f p1 = points[i];
        Vec3f p2 = points[i + 1];
        // 線分p1-p2に対する点positionの射影を計算
        // t = (position-p1)・(p2-p1) / |p2-p1|^2 で、線分上の位置を0〜1の比率として得る。
        // 制御点が重複していると分母が0になりゼロ除算するため、その区間は飛ばす
        Vec3f lineDir      = p2 - p1;
        float lineLengthSq = lineDir.lengthSq();
        if (lineLengthSq == 0.0f) {
            continue; // 同じ点の場合はスキップ
        }

        // clampすることで、線分の外側に射影された場合は最寄りの端点に丸められる。
        // これが無いと線分を無限直線として扱ってしまい、実際には遠い区間が最寄りと誤判定される
        float t = (Vec3f(position - p1).dot(lineDir)) / lineLengthSq;
        t       = std::clamp(t, 0.0f, 1.0f);

        Vec3f projection = p1 + lineDir * t;
        // 比較するだけなので平方根を取らず、2乗のまま比べて計算を省く
        float distanceSq = Vec3f(position - projection).lengthSq();
        // 最小距離を更新
        if (distanceSq < minDistanceSq) {
            minDistanceSq = distanceSq;
            closestIndex1 = static_cast<uint32_t>(i);
            closestIndex2 = static_cast<uint32_t>(i + 1);
        }
    }
    return {closestIndex1, closestIndex2};
}

std::pair<uint32_t, uint32_t> OriGine::CalcDistanceSegmentIndex(const std::deque<Vec3f>& points, float distance) {
    if (points.empty()) {
        return {0, 0};
    }
    uint32_t closestIndex1  = 0;
    uint32_t closestIndex2  = 1;
    float accumulatedLength = 0.0f;
    for (size_t i = 0; i < points.size() - 1; i++) {
        Vec3f p1            = points[i];
        Vec3f p2            = points[i + 1];
        float segmentLength = Vec3f(p2 - p1).length();
        if (accumulatedLength + segmentLength >= distance) {
            closestIndex1 = static_cast<uint32_t>(i);
            closestIndex2 = static_cast<uint32_t>(i + 1);
            break;
        }
        accumulatedLength += segmentLength;
    }
    return {closestIndex1, closestIndex2};
}

float OriGine::CalcDistanceAlongSpline(const std::deque<Vec3f>& points, const Vec3f& position) {
    if (points.empty()) {
        return 0.f;
    }
    float accumulatedLength = 0.0f;
    for (size_t i = 0; i < points.size() - 1; i++) {
        Vec3f p1            = points[i];
        Vec3f p2            = points[i + 1];
        float segmentLength = Vec3f(p2 - p1).length();
        // 線分p1-p2に対する点positionの射影を計算
        Vec3f lineDir      = p2 - p1;
        float lineLengthSq = lineDir.lengthSq();
        if (lineLengthSq == 0.0f) {
            continue; // 同じ点の場合はスキップ
        }
        float t          = (Vec3f(position - p1).dot(lineDir)) / lineLengthSq;
        t                = std::clamp(t, 0.0f, 1.0f);
        Vec3f projection = p1 + lineDir * t;
        // 射影点が線分内にある場合、距離を計算して返す
        // NOTE: 直前のclampでtは必ず0〜1に収まるため、この条件は常に真になる。
        //       結果としてこの関数は「最も近い区間」ではなく「先頭の区間」で必ず返る点に注意
        if (t >= 0.0f && t <= 1.0f) {
            return accumulatedLength + Vec3f(projection - p1).length();
        }
        accumulatedLength += segmentLength;
    }
    // 指定位置が全長を超える場合、全長を返す
    return accumulatedLength;
}

Vec3f OriGine::CalcPointOnSplineByDistance(const std::deque<Vec3f>& points, float distance) {
    if (points.empty()) {
        return Vec3f();
    }
    float accumulatedLength = 0.0f;
    for (size_t i = 0; i < points.size() - 1; i++) {
        Vec3f p1            = points[i];
        Vec3f p2            = points[i + 1];
        float segmentLength = Vec3f(p2 - p1).length();
        // 累積距離が目標距離を追い越した区間が、求める点を含む区間。
        // 区間内で余った分(remainingDistance)を区間長で割れば区間内の比率になる
        if (accumulatedLength + segmentLength >= distance) {
            float remainingDistance = distance - accumulatedLength;
            float t                 = remainingDistance / segmentLength;
            // 線形補間で点を計算
            return p1 + (p2 - p1) * t;
        }
        accumulatedLength += segmentLength;
    }
    // 指定距離が全長を超える場合、最後の点を返す
    return points.back();
}

#include "Quaternion.h"

/// stl
#include <algorithm>

/// math
#include <cmath>
#include <MathEnv.h>
#include <Matrix4x4.h>
#include <numbers>

namespace OriGine {

Quaternion Quaternion::Inverse(const Quaternion& q) {
    float normSq = q.normSq(); // ノルムの二乗
    if (normSq == 0.0f) {
        // ノルムが0の場合、逆元は存在しない
        return {0.0f, 0.0f, 0.0f, 0.0f}; // エラー処理としてゼロ四元数を返す
    }
    Quaternion conjugate = q.Conjugation();
    return Quaternion(conjugate / normSq); // 共役をノルムの二乗で割った結果を明示的にQuaternionへ変換
}

Quaternion Quaternion::inverse() const {
    float normSq = this->normSq(); // ノルムの二乗
    if (normSq == 0.0f) {
        // ノルムが0の場合、逆元は存在しない
        return {0.0f, 0.0f, 0.0f, 0.0f}; // エラー処理としてゼロ四元数を返す
    }
    Quaternion conjugate = this->Conjugation();
    return Quaternion(conjugate / normSq); // 明示的な変換
}

float Quaternion::Norm(const Quaternion& q) {
    return sqrtf((q[W] * q[W]) + (q[X] * q[X]) + (q[Y] * q[Y]) + (q[Z] * q[Z]));
}

float Quaternion::norm() const {
    return sqrtf((this->v[W] * this->v[W]) + (this->v[X] * this->v[X]) + (this->v[Y] * this->v[Y]) + (this->v[Z] * this->v[Z]));
}

float Quaternion::NormSq(const Quaternion& q) {
    return q[W] * q[W] + q[X] * q[X] + q[Y] * q[Y] + q[Z] * q[Z];
}

float Quaternion::normSq() const {
    return this->v[W] * this->v[W] + this->v[X] * this->v[X] + this->v[Y] * this->v[Y] + this->v[Z] * this->v[Z];
}

Quaternion Quaternion::Normalize(const Quaternion& q) {
    float norm = q.norm();
    if (norm == 0.0f) {
        return {0.0f, 0.0f, 0.0f, 1.0f};
    }
    return Quaternion(q / norm); // 明示的にQuaternionへ変換
}

Quaternion Quaternion::normalize() const {
    float norm = this->norm();
    if (norm == 0.0f) {
        return {0.0f, 0.0f, 0.0f, 1.0f};
    }
    return Quaternion(*this / norm); // 明示的にQuaternionへ変換
}

float Quaternion::Dot(const Quaternion& q0, const Quaternion& q1) {
    return q0[X] * q1[X] + q0[Y] * q1[Y] + q0[Z] * q1[Z] + q0[W] * q1[W];
}

float Quaternion::dot(const Quaternion& q) const {
    return v[X] * q[X] + v[Y] * q[Y] + v[Z] * q[Z] + v[W] * q[W];
}

/// <summary>
/// クォータニオンをオイラー角(ラジアン)に変換する。
/// </summary>
/// <remarks>
/// 各成分の式は、クォータニオンを回転行列に展開したときの要素比から
/// atan2/asinで角度を逆算したもの。行列を実際に作らずに済むよう整理されている。
/// 内部表現としてはクォータニオンのままの方が安全なので、この変換はGUI表示など
/// 人が角度を読み書きする場面に限って使うこと(往復させると誤差とジンバルロックで値がずれる)。
/// </remarks>
Vec3f Quaternion::ToEulerAngles() const {
    Vec3f euler;

    // Yaw (Y-axis rotation)
    float siny_cosp = 2.0f * (v[W] * v[Y] + v[X] * v[Z]);
    float cosy_cosp = 1.0f - 2.0f * (v[Y] * v[Y] + v[Z] * v[Z]);
    euler[Y]        = std::atan2(siny_cosp, cosy_cosp);

    // Pitch (X-axis rotation)
    // sinpは本来-1〜1に収まるが、浮動小数の誤差でわずかに超えることがある。
    // そのままasinに渡すと定義域外でNaNになるため、真上/真下を向いている(=ジンバルロック)
    // とみなして符号付きの90度(±π/2)を返す
    float sinp = 2.0f * (v[W] * v[X] - v[Y] * v[Z]);
    if (std::abs(sinp) >= 1.0f) {
        euler[X] = std::copysign(kHalfPi, sinp);
    } else {
        euler[X] = std::asin(sinp);
    }

    // Roll (Z-axis rotation)
    float sinr_cosp = 2.0f * (v[W] * v[Z] - v[X] * v[Y]);
    float cosr_cosp = 1.0f - 2.0f * (v[Z] * v[Z] + v[X] * v[X]);
    euler[Z]        = std::atan2(sinr_cosp, cosr_cosp);

    return euler;
}

float Quaternion::ToPitch() const {
    float sinp = 2.0f * (v[W] * v[X] - v[Y] * v[Z]);
    if (std::abs(sinp) >= 1.0f) {
        return std::copysign(kHalfPi, sinp);
    }
    return std::asin(sinp);
}

float Quaternion::ToYaw() const {
    float siny_cosp = 2.0f * (v[W] * v[Y] + v[X] * v[Z]);
    float cosy_cosp = 1.0f - 2.0f * (v[Y] * v[Y] + v[Z] * v[Z]);
    return std::atan2(siny_cosp, cosy_cosp);
}

float Quaternion::ToRoll() const {
    float sinr_cosp = 2.0f * (v[W] * v[Z] - v[X] * v[Y]);
    float cosr_cosp = 1.0f - 2.0f * (v[Z] * v[Z] + v[X] * v[X]);
    return std::atan2(sinr_cosp, cosr_cosp);
}

/// <summary>
/// クォータニオンqでベクトルvecを回転させる。
/// </summary>
/// <remarks>
/// ベクトルをW=0の純クォータニオンとみなし、q * v * q^-1 という「サンドイッチ積」で回転させる。
/// qが正規化されている前提のため、逆元の代わりに共役(Conjugation)を使っている
/// (正規化済みなら q^-1 == 共役 であり、除算を省ける)。
/// qが正規化されていない場合、結果はノルムの2乗倍に拡大される点に注意。
/// </remarks>
Vec3f Quaternion::RotateVector(const Vec3f& vec, const Quaternion& q) {
    Quaternion r = Quaternion(vec, 0.0f);
    r            = q * r * q.Conjugation();
    return Vec3f(r[X], r[Y], r[Z]);
}

Vec3f Quaternion::RotateVector(const Vec3f& vec) const {
    Quaternion r = Quaternion(vec, 0.0f);
    r            = *this * r * this->Conjugation();
    return Vec3f(r[X], r[Y], r[Z]);
}

/// <summary>
/// 指定軸まわりにangle(ラジアン)だけ回転するクォータニオンを作る。
/// </summary>
/// <remarks>
/// 角度を半分にするのは、上のRotateVectorがq*v*q^-1とqを2回掛ける形になっており、
/// 回転が二重に適用されるため。半角で作ることで実際の回転量がangleと一致する。
/// axisは正規化されている必要がある(していないと回転量が変わってしまう)。
/// </remarks>
Quaternion Quaternion::RotateAxisAngle(const Vec3f& axis, float angle) {
    float halfAngle = angle / 2.0f;
    return Quaternion(
        axis * sinf(halfAngle),
        cosf(halfAngle))
        .normalize();
}

/// <summary>
/// fromの向きをtoの向きへ合わせる回転を求める。
/// </summary>
/// <remarks>
/// 2ベクトルのなす角は内積のacos、その回転軸は両者に直交する外積で得られる。
/// WARNING: from・toが正規化されている前提で、内積のclampを行っていない。
///          誤差で内積が±1をわずかに超えるとacosがNaNを返し、
///          また2ベクトルが平行/真逆の場合は外積がゼロになりnormalizeが破綻する。
///          呼び出し側でこれらが起きないことを保証できない場合は、
///          同等の処理をより安全に行うFromNormalVector()を使うこと。
/// </remarks>
const Quaternion Quaternion::RotateAxisVector(const Vec3f& from, const Vec3f& to) {
    float angle = std::acosf(from.dot(to));
    Vec3f axis  = from.cross(to).normalize();

    float halfAngle = angle / 2.0f;
    return Quaternion(
        axis * sinf(halfAngle),
        cosf(halfAngle))
        .normalize();
}

const Quaternion Quaternion::FromNormalVector(const Vec3f& normal, const Vec3f& up) {
    Vec3f from = up.normalize();
    Vec3f to   = normal.normalize();

    float dot = from.dot(to);
    // 数値誤差対策
    dot = std::clamp(dot, -1.0f, 1.0f);

    if (std::abs(dot - 1.0f) < kEpsilon) {
        // ほぼ同じ方向 → 回転不要
        return Quaternion::Identity();
    }

    if (std::abs(dot + 1.0f) < kEpsilon) {
        // 真逆方向 → 180度回転
        // from と直交する任意の軸を求める
        Vec3f axis = Vec3f::Cross(from, Vec3f(1.0f, 0.0f, 0.0f));
        if (axis.lengthSq() < kEpsilon) {
            // 万一 from が (1,0,0) と平行なら別の軸を選ぶ
            axis = Vec3f::Cross(from, Vec3f(0.0f, 1.0f, 0.0f));
        }
        axis = axis.normalize();
        return Quaternion::RotateAxisAngle(axis, std::numbers::pi_v<float>);
    }

    Vec3f axis  = Vec3f::Cross(from, to).normalize();
    float angle = std::acos(dot);
    return Quaternion::RotateAxisAngle(axis, angle);
}

/// <summary>
/// 回転行列からクォータニオンを復元する。
/// </summary>
/// <remarks>
/// 4つの成分はいずれも行列要素から平方根で求められるが、対象の成分が0に近いと
/// 平方根の中身が0に近づき、除算で誤差が大きく増幅される。
/// そこで「最も絶対値が大きくなる成分」を先に選んで基準にし、残りをその成分で割って求める
/// (対角成分の和traceと各対角成分を比較しているのがその選択にあたる)。
/// どの分岐でも計算結果は同じ回転を表し、数値的な安定性だけが違う。
/// </remarks>
Quaternion Quaternion::FromMatrix(const Matrix4x4& rotateMat) {
    // traceが正なら W が最大になるので、W を基準に他の3成分を求められる
    float trace = rotateMat.m[0][0] + rotateMat.m[1][1] + rotateMat.m[2][2];

    if (trace > 0.0f) {
        float s    = std::sqrt(trace + 1.0f) * 2.0f;
        float invS = 1.0f / s;

        return Quaternion(
            (rotateMat.m[2][1] - rotateMat.m[1][2]) * invS, // x
            (rotateMat.m[0][2] - rotateMat.m[2][0]) * invS, // y
            (rotateMat.m[1][0] - rotateMat.m[0][1]) * invS, // z
            0.25f * s // w
        );
    } else {
        // traceが0以下＝Wが小さいので、対角成分が最大の軸(X/Y/Z)を基準に切り替える
        if (rotateMat.m[0][0] > rotateMat.m[1][1] && rotateMat.m[0][0] > rotateMat.m[2][2]) {
            float s    = std::sqrt(1.0f + rotateMat.m[0][0] - rotateMat.m[1][1] - rotateMat.m[2][2]) * 2.0f;
            float invS = 1.0f / s;

            return Quaternion(
                0.25f * s,
                (rotateMat.m[0][1] + rotateMat.m[1][0]) * invS,
                (rotateMat.m[0][2] + rotateMat.m[2][0]) * invS,
                (rotateMat.m[2][1] - rotateMat.m[1][2]) * invS);
        } else if (rotateMat.m[1][1] > rotateMat.m[2][2]) {
            float s    = std::sqrt(1.0f + rotateMat.m[1][1] - rotateMat.m[0][0] - rotateMat.m[2][2]) * 2.0f;
            float invS = 1.0f / s;

            return Quaternion(
                (rotateMat.m[0][1] + rotateMat.m[1][0]) * invS,
                0.25f * s,
                (rotateMat.m[1][2] + rotateMat.m[2][1]) * invS,
                (rotateMat.m[0][2] - rotateMat.m[2][0]) * invS);
        } else {
            float s    = std::sqrt(1.0f + rotateMat.m[2][2] - rotateMat.m[0][0] - rotateMat.m[1][1]) * 2.0f;
            float invS = 1.0f / s;

            return Quaternion(
                (rotateMat.m[0][2] + rotateMat.m[2][0]) * invS,
                (rotateMat.m[1][2] + rotateMat.m[2][1]) * invS,
                0.25f * s,
                (rotateMat.m[1][0] - rotateMat.m[0][1]) * invS);
        }
    }
}

Quaternion Quaternion::FromEulerAngles(float pitch, float yaw, float roll) {
    // 半分の角度を計算
    float halfPitch = pitch * 0.5f;
    float halfYaw   = yaw * 0.5f;
    float halfRoll  = roll * 0.5f;

    // サインとコサインを計算
    float sinPitch = sin(halfPitch);
    float cosPitch = cos(halfPitch);
    float sinYaw   = sin(halfYaw);
    float cosYaw   = cos(halfYaw);
    float sinRoll  = sin(halfRoll);
    float cosRoll  = cos(halfRoll);

    // クォータニオンを計算
    Quaternion q;
    q[X] = cosYaw * sinPitch * cosRoll + sinYaw * cosPitch * sinRoll;
    q[Y] = sinYaw * cosPitch * cosRoll - cosYaw * sinPitch * sinRoll;
    q[Z] = cosYaw * cosPitch * sinRoll - sinYaw * sinPitch * cosRoll;
    q[W] = cosYaw * cosPitch * cosRoll + sinYaw * sinPitch * sinRoll;

    return q;
}

Quaternion Quaternion::FromEulerAngles(const Vec3f& euler) {
    return FromEulerAngles(euler[X], euler[Y], euler[Z]);
}

/// <summary>
/// ローカルZ軸をforwardの向きへ合わせる回転を返す。
/// upは「どちらを上とするか」の目安で、forwardと直交している必要はない。
/// </summary>
Quaternion Quaternion::LookAt(const Vec3f& forward, const Vec3f& up) { // Z軸を向けるべき方向にする
    Vec3f forwardV = Vec3f::Normalize(forward);

    // 右ベクトルを計算（外積）
    Vec3f right = Vec3f::Normalize(Vec3f::Cross(up, forwardV));

    // 上ベクトルを再計算
    // 引数のupはforwardと直交しているとは限らないため、そのまま基底にすると
    // 行列が歪んでしまう。forwardとrightの外積を取り直すことで、
    // 3軸が必ず直交する正しい回転行列になる(グラム・シュミット直交化に相当)
    Vec3f newUp = Vec3f::Cross(forwardV, right);

    // 回転行列を作成
    Matrix4x4 lookAtMatrix = {
        right[X], newUp[X], forwardV[X], 0.0f,
        right[Y], newUp[Y], forwardV[Y], 0.0f,
        right[Z], newUp[Z], forwardV[Z], 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f};

    // 行列からクォータニオンに変換
    return FromMatrix(lookAtMatrix);
};

constexpr Quaternion operator*(float scalar, const Quaternion& q) {
    return Quaternion(q * scalar);
}

constexpr Quaternion operator/(float scalar, const Quaternion& q) {
    return Quaternion(q / scalar);
}

/// <summary>
/// 2つの回転を球面線形補間する。回転を一定の角速度で滑らかに繋ぐ。
/// </summary>
/// <remarks>
/// qと-qは同じ姿勢を表すため、そのまま補間すると意図せず遠回り(最大360度近く)する
/// ことがある。内積の符号でこれを検出し、片方を反転させて必ず短い方の弧を通す。
/// </remarks>
Quaternion Slerp(const Quaternion& q0, const Quaternion& q1, float t) {
    float dot = q0.dot(q1);

    // ドット積が負の場合、q1 を反転して最短経路を取る
    // (内積が負＝なす角が90度超＝逆回りの方が近い、という判定)
    Quaternion q1Adjusted = q1;
    if (dot < 0.0f) {
        q1Adjusted = -q1Adjusted;
        dot        = -dot;
    }

    // θがほぼゼロの場合、直接返す
    // 2つの回転がほぼ同じだとthetaが0に近づき、下のsinThetaでの除算が
    // ゼロ除算になって結果が発散する。角度が十分小さければ球面上の弧と直線の差は
    // 無視できるので、安全な線形補間で代用する。
    // 0.9995という閾値は、float精度でsinThetaが破綻し始める手前の経験値
    if (dot > 0.9995f) {
        return Quaternion(q0 * (1.0f - t) + q1Adjusted * t).normalize(); // 線形補間を用いる
    }

    float theta    = acosf(dot);
    float sinTheta = sinf(theta);

    float scale0 = sinf((1.0f - t) * theta) / sinTheta;
    float scale1 = sinf(t * theta) / sinTheta;

    return Quaternion(q0 * scale0 + q1Adjusted * scale1).normalize(); // 結果を正規化
}

} // namespace OriGine

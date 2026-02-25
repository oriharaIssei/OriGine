#pragma once

/// stl
#include <algorithm>
#include <type_traits>

/// math
#include <cmath>

/// externals
#include <logger/Logger.h>
#include <nlohmann/json.hpp>

namespace OriGine {
// indexNumbers
enum AxisIndex : int {
    X = 0,
    Y = 1,
    Z = 2,
    W = 3
};
enum ColorChannel {
    R = 0,
    G,
    B,
    A,
    MAX = 4
};

template <typename valueType>
struct Vector2;
template <typename valueType>
struct Vector3;
template <typename valueType>
struct Vector4;

/// <summary>
/// ベクトル構造体
/// </summary>
/// <typeparam name="valueType">数値クラス</typeparam>
/// <typeparam name="dimension">次元</typeparam>
template <int dimension, typename valueType>
struct Vector {

    // デフォルトコンストラクタ
    constexpr Vector() : v{} {} // ゼロ初期化

    // コピーコンストラクタ
    constexpr Vector(const Vector& other) {
        for (int i = 0; i < dimension; i++) {
            v[i] = other.v[i];
        }
    }

    // 配列からのコンストラクタ
    constexpr Vector(const valueType* _ptr) {
        for (int i = 0; i < dimension; i++) {
            v[i] = _ptr[i];
        }
    }

    // 可変長引数コンストラクタ
    template <typename... Args>
    constexpr Vector(Args... args) : v{static_cast<valueType>(args)...} {
        static_assert(sizeof...(args) == dimension,
            "The number of arguments must be equal to the dimension of the vector.");
    }

public:
    // メンバ変数
    static constexpr int dim = dimension;
    valueType v[dim];

public:
    // 演算子のオーバーロード
    // accessor
    constexpr valueType& operator[](std::size_t i) { return v[i]; }
    constexpr const valueType& operator[](std::size_t i) const { return v[i]; }
    constexpr valueType& operator[](AxisIndex i) { return v[i]; }
    constexpr const valueType& operator[](AxisIndex i) const { return v[i]; }

    // plus
    constexpr Vector operator+(const Vector& other) const {
        Vector result;
        for (int i = 0; i < dim; i++) {
            result.v[i] = v[i] + other.v[i];
        }
        return result;
    }
    Vector& operator+=(const Vector& other) {
        for (int i = 0; i < dim; i++) {
            v[i] += other.v[i];
        }
        return *this;
    }
    // minus
    constexpr Vector operator-(const Vector& other) const {
        Vector result;
        for (int i = 0; i < dim; i++) {
            result.v[i] = v[i] - other.v[i];
        }

        return result;
    }
    constexpr Vector operator-() const {
        Vector result;
        for (int i = 0; i < dim; i++) {
            result.v[i] = -v[i];
        }
        return result;
    }
    Vector& operator-=(const Vector& other) {
        for (int i = 0; i < dim; i++) {
            v[i] -= other.v[i];
        }
        return *this;
    }
    // multiply
    constexpr Vector operator*(const valueType& scalar) const {
        Vector result;
        for (int i = 0; i < dim; i++) {
            result.v[i] = v[i] * scalar;
        }
        return result;
    }
    Vector& operator*=(const Vector& other) {
        for (int i = 0; i < dim; i++) {
            v[i] *= other.v[i];
        }
        return *this;
    }
    Vector& operator*=(const valueType& scalar) {
        for (int i = 0; i < dim; i++) {
            v[i] *= scalar;
        }
        return *this;
    }
    // divide
    constexpr Vector operator/(const valueType& scalar) const {
        Vector result;
        if (scalar != 0) {
            for (int i = 0; i < dim; i++)
                result.v[i] = v[i] / scalar;
        } else {
            for (int i = 0; i < dim; i++)
                result.v[i] = 0;
        }
        return result;
    }
    Vector& operator/=(const valueType& scalar) {
        if (scalar != 0) {
            for (int i = 0; i < dim; i++)
                v[i] /= scalar;
        } else {
            for (int i = 0; i < dim; i++)
                v[i] = 0;
        }
        return *this;
    }
    Vector& operator/=(const Vector& other) {
        for (int i = 0; i < dim; i++) {
            if (other.v[i] != 0) {
                v[i] /= other.v[i];
            } else {
                v[i] = 0;
            }
        }
        return *this;
    }

    // equal
    constexpr bool operator==(const Vector& other) const {
        for (int i = 0; i < dim; i++)
            if (v[i] != other.v[i])
                return false;
        return true;
    }
    // not equal
    constexpr bool operator!=(const Vector& other) const {
        return !(*this == other);
    }

    constexpr bool operator<(const Vector& other) const {
        for (int i = 0; i < dim; i++)
            if (v[i] >= other.v[i])
                return false;
        return true;
    }
    constexpr bool operator>(const Vector& other) const {
        for (int i = 0; i < dim; i++)
            if (v[i] <= other.v[i])
                return false;
        return true;
    }

    constexpr Vector& operator=(const Vector& other) {
        for (int i = 0; i < dim; i++)
            v[i] = other.v[i];
        return *this;
    }

    // 数学系

    /// <summary>
    /// ベクトルの長さを計算
    /// </summary>
    /// <returns>ベクトルの長さ</returns>
    constexpr valueType length() const;
    /// <summary>
    /// ベクトルの長さを計算 (static)
    /// </summary>
    /// <param name="_v">ベクトル</param>
    /// <returns>ベクトルの長さ</returns>
    static constexpr valueType Length(const Vector& _v);

    /// <summary>
    /// ベクトルの長さの2乗を計算
    /// </summary>
    constexpr valueType lengthSq() const;
    /// <summary>
    /// ベクトルの長さの2乗を計算 (static)
    /// </summary>
    /// <param name="_v1">ベクトル</param>
    /// <returns>長さの2乗</returns>
    static constexpr valueType LengthSq(const Vector& _v1);

    /// <summary>
    /// 内積を計算
    /// </summary>
    constexpr valueType dot() const;
    constexpr valueType dot(const Vector& _v) const;

    /// <summary>
    /// 内積を計算 (static)
    /// </summary>
    /// <param name="_v">ベクトル</param>
    /// <returns>内積</returns>
    static constexpr valueType Dot(const Vector& _v);
    /// <summary>
    /// 内積を計算 (static)
    /// </summary>
    /// <param name="_v"></param>
    /// <param name="_another"></param>
    /// <returns></returns>
    static constexpr valueType Dot(const Vector& _v, const Vector& _another);

    /// <summary>
    /// 正規化
    /// </summary>
    /// <returns>正規化済みベクトル</returns>
    constexpr Vector normalize() const;
    /// <summary>
    /// 正規化(static)
    /// </summary>
    /// <param name="_v">正規化 前</param>
    /// <returns>正規化 後</returns>
    static constexpr Vector Normalize(const Vector& _v);

    // 型変換
    constexpr operator Vector2<valueType>() const {
        static_assert(dim == 2, "Conversion only available for 2D vectors.");
        return Vector2<valueType>(v[X], v[Y]);
    }
    constexpr operator Vector3<valueType>() const {
        static_assert(dim == 3, "Conversion only available for 3D vectors.");
        return Vector3<valueType>(v[X], v[Y], v[Z]);
    }
    constexpr operator Vector4<valueType>() const {
        static_assert(dim == 4, "Conversion only available for 4D vectors.");
        return Vector4<valueType>(v[X], v[Y], v[Z], v[W]);
    }
};

template <int dim, typename valueType>
inline Vector<dim, valueType> operator*(const valueType& scalar, const Vector<dim, valueType>& vec) {
    Vector<dim, valueType> result;
    for (int i = 0; i < dim; i++) {
        result[i] = vec[i] * scalar;
    }
    return result;
}

template <int dim, typename valueType>
inline Vector<dim, valueType> operator*(const Vector<dim, valueType>& vec, const Vector<dim, valueType>& another) {
    Vector<dim, valueType> result;
    for (int i = 0; i < dim; i++) {
        result[i] = vec[i] * another[i];
    }
    return result;
}
template <int dim, typename valueType>
inline Vector<dim, valueType>* operator*=(Vector<dim, valueType>& vec, const Vector<dim, valueType>& another) {
    for (int i = 0; i < dim; i++) {
        vec[i] *= another[i];
    }
    return &vec;
}

template <int dim, typename valueType>
inline Vector<dim, valueType> operator/(const Vector<dim, valueType>& vec, const Vector<dim, valueType>& another) {
    Vector<dim, valueType> result;
    for (int i = 0; i < dim; i++) {
        if (another[i] != 0) {
            result[i] = vec[i] / another[i];
        } else {
            result[i] = 0;
        }
    }
    return result;
}
template <int dim, typename valueType>
inline Vector<dim, valueType>* operator/=(Vector<dim, valueType>& vec, const Vector<dim, valueType>& another) {
    for (int i = 0; i < dim; i++) {
        if (another[i] != 0) {
            vec[i] /= another[i];
        } else {
            vec[i] = 0;
        }
    }
    return &vec;
}

template <int dim, typename valueType>
inline Vector<dim, valueType> Lerp(const Vector<dim, valueType>& start, const Vector<dim, valueType>& end, float t) {
    return start + (end - start) * t;
}
template <int dim, typename valueType>
inline Vector<dim, valueType> lerp(const Vector<dim, valueType>& start, const Vector<dim, valueType>& end, float t) {
    return start + (end - start) * t;
}

/// <summary>
/// 要素ごとの最小値を計算
/// </summary>
/// <typeparam name="valueType"></typeparam>
/// <typeparam name="dim"></typeparam>
/// <param name="a"></param>
/// <param name="b"></param>
/// <returns></returns>
template <int dim, typename valueType>
inline Vector<dim, valueType> MinElement(const Vector<dim, valueType>& a, const Vector<dim, valueType>& b) {
    Vector<dim, valueType> result;
    for (int32_t i = 0; i < dim; ++i) {
        result[i] = (std::min)(a[i], b[i]);
    }
    return result;
}
/// <summary>
/// 要素ごとの最大値を計算
/// </summary>
/// <typeparam name="valueType"></typeparam>
/// <typeparam name="dim"></typeparam>
/// <param name="a"></param>
/// <param name="b"></param>
/// <returns></returns>
template <int dim, typename valueType>
inline Vector<dim, valueType> MaxElement(const Vector<dim, valueType>& a, const Vector<dim, valueType>& b) {
    Vector<dim, valueType> result;
    for (int32_t i = 0; i < dim; ++i) {
        result[i] = (std::max)(a[i], b[i]);
    }
    return result;
}

template <int dimension, typename valueType>
using Vec = Vector<dimension, valueType>;

/// <summary>
/// nlohmann::json へのシリアライズ (Vector)
/// </summary>
template <int dim, typename valueType>
inline void to_json(nlohmann::json& j, const Vector<dim, valueType>& v) {
    // VectorをJSON配列としてシリアライズ
    j = nlohmann::json::array();
    for (int i = 0; i < dim; ++i) {
        j.push_back(v[i]);
    }
}

/// <summary>
/// nlohmann::json からのデシリアライズ (Vector)
/// </summary>
template <int dim, typename valueType>
inline void from_json(const nlohmann::json& j, Vector<dim, valueType>& v) {
    // JSON配列をVectorにデシリアライズ
    if (!j.is_array() || j.size() != dim) {
        for (int i = 0; i < dim; ++i) {
            v[i] = valueType(0);
        }
        LOG_ERROR("JSON array size does not match Vector dimension. content: {}", j.dump());
        return;
    }
    for (int i = 0; i < dim; ++i) {
        v[i] = j.at(i).get<valueType>();
    }
}

template <int dimension, typename valueType>
inline constexpr valueType Vector<dimension, valueType>::length() const {
    valueType sum = 0;
    for (int i = 0; i < dim; i++) {
        sum += v[i] * v[i];
    }
    return std::sqrt(sum);
}

template <int dimension, typename valueType>
inline constexpr valueType Vector<dimension, valueType>::Length(const Vector& _v) {
    valueType sum = 0;
    for (int i = 0; i < dim; i++) {
        sum += _v.v[i] * _v.v[i];
    }
    return std::sqrt(sum);
}

template <int dimension, typename valueType>
inline constexpr valueType Vector<dimension, valueType>::lengthSq() const {
    valueType sum = 0;
    for (int i = 0; i < dim; i++) {
        sum += v[i] * v[i];
    }
    return sum;
}

template <int dimension, typename valueType>
inline constexpr valueType Vector<dimension, valueType>::LengthSq(const Vector& _v1) {
    valueType sum = 0;
    for (int i = 0; i < dim; i++) {
        sum += _v1.v[i] * _v1.v[i];
    }
    return sum;
}

template <int dimension, typename valueType>
inline constexpr valueType Vector<dimension, valueType>::dot() const {
    valueType sum = 0;
    for (int i = 0; i < dim; i++) {
        sum += v[i] * v[i];
    }
    return sum;
}

template <int dimension, typename valueType>
inline constexpr valueType Vector<dimension, valueType>::dot(const Vector& _v) const {
    valueType sum = 0;
    for (int i = 0; i < dim; i++) {
        sum += v[i] * _v.v[i];
    }
    return sum;
}

template <int dimension, typename valueType>
inline constexpr valueType Vector<dimension, valueType>::Dot(const Vector& _v) {
    valueType sum = 0;
    for (int i = 0; i < dim; i++) {
        sum += _v.v[i] * _v.v[i];
    }
    return sum;
}

template <int dimension, typename valueType>
inline constexpr valueType Vector<dimension, valueType>::Dot(const Vector& _v, const Vector& _another) {
    valueType sum = 0;
    for (int i = 0; i < dim; i++) {
        sum += _v.v[i] * _another.v[i];
    }
    return sum;
}

template <int dimension, typename valueType>
inline constexpr Vector<dimension, valueType> Vector<dimension, valueType>::normalize() const {
    valueType len = length();
    if (len == 0) {
        return *this;
    }
    return (*this / len);
}

template <int dimension, typename valueType>
inline constexpr Vector<dimension, valueType> Vector<dimension, valueType>::Normalize(const Vector<dimension, valueType>& _v) {
    valueType len = _v.length();
    if (len == 0) {
        return _v;
    }
    return (_v / len);
}

} // namespace OriGine

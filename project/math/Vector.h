#pragma once

/// stl
#include <algorithm>
#include <type_traits>

/// math
#include <cmath>

/// externals
#include <nlohmann/json.hpp>

// indexNumbers
static constexpr int X = 0;
static constexpr int Y = 1;
static constexpr int Z = 2;
static constexpr int W = 3;

static constexpr int R = 0;
static constexpr int G = 1;
static constexpr int B = 2;
static constexpr int A = 3;

template <typename valueType>
struct Vector2;
template <typename valueType>
struct Vector3;
template <typename valueType>
struct Vector4;

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

    // plus
    constexpr Vector operator+(const Vector& other) const {
        Vector result;
        for (int i = 0; i < dim; i++) {
            result.v[i] = v[i] + other.v[i];
        }
        return result;
    }
    Vector* operator+=(const Vector& other) {
        for (int i = 0; i < dim; i++) {
            v[i] += other.v[i];
        }
        return this;
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
    Vector* operator-=(const Vector& other) {
        for (int i = 0; i < dim; i++) {
            v[i] -= other.v[i];
        }
        return this;
    }
    // multiply
    constexpr Vector operator*(const valueType& scalar) const {
        Vector result;
        for (int i = 0; i < dim; i++) {
            result.v[i] = v[i] * scalar;
        }
        return result;
    }
    Vector* operator*=(const Vector& other) {
        for (int i = 0; i < dim; i++) {
            v[i] *= other.v[i];
        }
        return this;
    }
    Vector* operator*=(const valueType& scalar) {
        for (int i = 0; i < dim; i++) {
            v[i] *= scalar;
        }
        return this;
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
    Vector* operator/=(const valueType& scalar) {
        if (scalar != 0) {
            for (int i = 0; i < dim; i++)
                v[i] /= scalar;
        } else {
            for (int i = 0; i < dim; i++)
                v[i] = 0;
        }
        return this;
    }
    Vector* operator/=(const Vector& other) {
        for (int i = 0; i < dim; i++) {
            if (other.v[i] != 0) {
                v[i] /= other.v[i];
            } else {
                v[i] = 0;
            }
        }
        return this;
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

template <int dim, typename valueType>
inline Vector<dim, valueType> MinElement(const Vector<dim, valueType>& a, const Vector<dim, valueType>& b) {
    Vector<dim, valueType> result;
    for (int32_t i = 0; i < dim; ++i) {
        result[i] = (std::min)(a[i], b[i]);
    }
    return result;
}
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

template <int dim, typename valueType>
inline void to_json(nlohmann::json& j, const Vector<dim, valueType>& v) {
    // VectorをJSON配列としてシリアライズ
    j = nlohmann::json::array();
    for (int i = 0; i < dim; ++i) {
        j.push_back(v[i]);
    }
}

template <int dim, typename valueType>
inline void from_json(const nlohmann::json& j, Vector<dim, valueType>& v) {
    // JSON配列をVectorにデシリアライズ
    if (!j.is_array() || j.size() != dim) {
        throw std::invalid_argument("JSON array size does not match Vector dimension");
    }
    for (int i = 0; i < dim; ++i) {
        v[i] = j.at(i).get<valueType>();
    }
}

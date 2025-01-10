#pragma once

template <int dimension, typename valueType>
struct Vector {
    Vector() {
        for (int i = 0; i < dimension; i++)
            v[i] = 0;
    }

    Vector(const Vector& other) {
        for (int i = 0; i < dimension; i++)
            v[i] = other.v[i];
    }
    template <typename... Args>
    Vector(Args... args) {
        // 初期化子と Vector の次元が一致しているかをチェック
        static_assert(sizeof...(args) == dimension, "The number of arguments must be equal to the dimension of the vector.");
        valueType argArray[] = {args...};
        for (int i = 0; i < dimension; i++)
            v[i] = argArray[i];
    }

public:
    //メンバ変数
    static const int dim = dimension;
    valueType v[dimension];

public:
    //演算子のオーバーロード
    //accessor
    valueType& operator[](int index) { return v[index]; }
    const valueType& operator[](int index) const { return v[index]; }

    // plus
    Vector operator+(const Vector& other) const {
        Vector result;
        for (int i = 0; i < dimension; i++) {
            result.v[i] = v[i] + other.v[i];
        }
        return result;
    }
    Vector* operator+=(const Vector& other) {
        for (int i = 0; i < dimension; i++) {
            v[i] += other.v[i];
        }
        return this;
    }
    // minus
    Vector operator-(const Vector& other) const {
        Vector result;
        for (int i = 0; i < dimension; i++) {
            result.v[i] = v[i] - other.v[i];
        }

        return result;
    }
    Vector operator-() const {
        Vector result;
        for (int i = 0; i < dimension; i++) {
            result.v[i] = -v[i];
        }
        return result;
    }
    Vector* operator-=(const Vector& other) {
        for (int i = 0; i < dimension; i++) {
            v[i] -= other.v[i];
        }
        return this;
    }
    // multiply
    Vector operator*(const valueType& scalar) const {
        Vector result;
        for (int i = 0; i < dimension; i++) {
            result.v[i] = v[i] * scalar;
        }
        return result;
    }
    Vector* operator*=(const valueType& scalar) {
        for (int i = 0; i < dimension; i++) {
            v[i] *= scalar;
        }
        return this;
    }
    // divide
    Vector operator/(const valueType& scalar) const {
        Vector result;
        if (scalar != 0) {
            for (int i = 0; i < dimension; i++)
                result.v[i] = v[i] / scalar;
        } else {
            for (int i = 0; i < dimension; i++)
                result.v[i] = 0;
        }
        return result;
    }
    Vector* operator/=(const valueType& scalar) {
        if (scalar != 0) {
            for (int i = 0; i < dimension; i++)
                v[i] /= scalar;
        } else {
            for (int i = 0; i < dimension; i++)
                v[i] = 0;
        }
        return this;
    }

    // equal
    bool operator==(const Vector& other) const {
        for (int i = 0; i < dimension; i++)
            if (v[i] != other.v[i]) return false;
        return true;
    }
    // not equal
    bool operator!=(const Vector& other) const {
        return !(*this == other);
    }
};

template <int dimension, typename valueType>
using Vec = Vector<dimension, valueType>;

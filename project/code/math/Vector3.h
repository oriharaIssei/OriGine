#pragma once

#include "Vector2.h"

#include <cmath>

struct Vector3 final{
    float x,y,z;

    // コンストラクタ
    Vector3(float xValue,float yValue,float zValue): x(xValue),y(yValue),z(zValue){}
    Vector3(int X,int Y,int Z): x((float)X),y((float)Y),z((float)Z){}
    Vector3(const Vector2& xy,const float& z):x(xy.x),y(xy.y),z(z){}
    Vector3(const float& x,const Vector2& yz):x(x),y(yz.x),z(yz.y){}
    Vector3(const float* x_ptr,const float* y_ptr,const float* z_ptr): x(*x_ptr),y(*y_ptr),z(*z_ptr){}
    Vector3(const float* ptr): x(ptr[0]),y(ptr[1]),z(ptr[2]){}
    Vector3(): x(0.0f),y(0.0f),z(0.0f){}

    // ベクトルの長さ
    float length() const{ return sqrtf(x * x + y * y + z * z); }
    static float Length(const Vector3& v){ return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z); }

    // ベクトルの長さの二乗
    float lengthSq() const{ return (x * x + y * y + z * z); }
    static float LengthSq(const Vector3& v){ return (v.x * v.x + v.y * v.y + v.z * v.z); }

    // 内積
    float dot(const Vector3& another) const{
        return (x * another.x) + (y * another.y) + (z * another.z);
    }
    static float Dot(const Vector3& v,const Vector3& another){
        return (v.x * another.x) + (v.y * another.y) + (v.z * another.z);
    }

    // 外積
    Vector3 cross(const Vector3& another) const{
        return Vector3(
            y * another.z - z * another.y,
            z * another.x - x * another.z,
            x * another.y - y * another.x
        );
    }
    static Vector3 Cross(const Vector3& v,const Vector3& another){
        return v.cross(another);
    }

    // 正規化
    Vector3 normalize() const{
        float len = length();
        if(len == 0) return *this;
        return (*this / len);
    }
    static Vector3 Normalize(const Vector3& v){
        float len = v.length();
        if(len == 0) return v;
        return (v / len);
    }

    // 演算子オーバーロード（+ - * / 等）
    Vector3 operator+(const Vector3& other) const{
        return Vector3(x + other.x,y + other.y,z + other.z);
    }
    void operator+=(const Vector3& other){
        x += other.x;
        y += other.y;
        z += other.z;
    }

    Vector3 operator-(const Vector3& other) const{
        return Vector3(x - other.x,y - other.y,z - other.z);
    }
    Vector3 operator-() const{ return Vector3(-x,-y,-z); }
    void operator-=(const Vector3& other){
        x -= other.x;
        y -= other.y;
        z -= other.z;
    }

    Vector3 operator*(const float& scalar) const{
        return Vector3(x * scalar,y * scalar,z * scalar);
    }
    void operator*=(const float& scalar){
        x *= scalar;
        y *= scalar;
        z *= scalar;
    }

    Vector3 operator/(const float& scalar) const{
        if(scalar != 0){
            return Vector3(x / scalar,y / scalar,z / scalar);
        } else{
            return Vector3(0.0f,0.0f,0.0f);
        }
    }
    void operator/=(const float& scalar){
        if(scalar != 0){
            x /= scalar;
            y /= scalar;
            z /= scalar;
        } else{
            x = 0;
            y = 0;
            z = 0;
        }
    }

    bool operator==(const Vector3& other) const{
        return (x == other.x && y == other.y && z == other.z);
    }
    bool operator!=(const Vector3& other) const{
        return !(*this == other);
    }
};

inline Vector3 operator*(const float& scalar,const Vector3& vec){
    return Vector3(vec.x * scalar,vec.y * scalar,vec.z * scalar);
}

inline Vector3 Lerp(const Vector3& start,const Vector3& end,float time){
    return {
        std::lerp(start.x,end.x,time),
        std::lerp(start.y,end.y,time),
        std::lerp(start.z,end.z,time)
    };
}

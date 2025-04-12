#pragma once

/// stl
#include <memory>

/// engine
// dreictX12
#include "directX12/Mesh.h"

/// math
#include <numbers>
#include <Vector2.h>
#include <Vector3.h>
#include <Vector4.h>

enum class PrimitiveType {
    Plane, // 面
    Circle, // 円
    Ring, // 環(真ん中が空洞)
    Sphere, // 球
    Torus, // トーラス
    Cylinder, // 円柱
    Cone, // 円錐
    Triangel, // 三角形
    Box // 立方体
};

/// <summary>
/// 形状の基底クラス
/// </summary>
class IPrimitive {
public:
    IPrimitive(PrimitiveType _type) : type_(_type) {}
    virtual ~IPrimitive() {
        mesh_.reset();
    }

    virtual void Initialize() {
        if (mesh_ == nullptr) {
            createMesh();
        }
    }
    virtual void createMesh() = 0;

protected:
    std::shared_ptr<TextureMesh> mesh_ = nullptr;

private:
    PrimitiveType type_;

public: // accessor
    PrimitiveType getType() const { return type_; }

    const std::shared_ptr<TextureMesh>& getMesh() const {
        return mesh_;
    }
};

class Plane
    : public IPrimitive {
public:
    Plane() : IPrimitive(PrimitiveType::Plane) {}
    ~Plane() override {
        mesh_.reset();
    }
    void Initialize() override {
        // mesh Init
        mesh_ = std::make_shared<TextureMesh>();
        mesh_->setName("Plane");
        mesh_->Initialize(4, 6);

        // create mesh
        createMesh();
    }

    void createMesh() override {
        mesh_->setVertexData({{Vec4f(-size_[X], 0.0f, -size_[Y], 1.0f), Vec2f(0.0f, 0.0f), normal_},
            {Vec4f(size_[X], 0.0f, -size_[Y], 1.0f), Vec2f(uv_[X], 0.0f), normal_},
            {Vec4f(-size_[X], 0.0f, size_[Y], 1.0f), Vec2f(0.0f, uv_[Y]), normal_},
            {Vec4f(size_[X], 0.0f, size_[Y], 1.0f), Vec2f(uv_[X], uv_[Y]), normal_}});
        mesh_->setIndexData({2, 3, 1,
            2, 1, 0});
        mesh_->TransferData();
    }

private:
    Vec2f size_   = {1.0f, 1.0f};
    Vec2f uv_     = {1.0f, 1.0f};
    Vec3f normal_ = {0.0f, 0.0f, 1.0f};

public: // accessor
    const Vec2f& getSize() const {
        return size_;
    }
    void setSize(const Vec2f& _size) {
        size_ = _size;
    }
    const Vec2f& getUV() const {
        return uv_;
    }
    void setUV(const Vec2f& _uv) {
        uv_ = _uv;
    }
    const Vec3f& getNormal() const {
        return normal_;
    }
    void setNormal(const Vec3f& _normal) {
        normal_ = _normal;
    }
};

class Circle
    : public IPrimitive {
public:
    Circle() : IPrimitive(PrimitiveType::Circle) {}
    ~Circle() override {
        mesh_.reset();
    }
    void Initialize() override {
        // mesh Init
        mesh_ = std::make_shared<TextureMesh>();
        mesh_->setName("Circle");
        mesh_->Initialize(vertexSize_, indexSize_);
        // create mesh
        createMesh();
    }
    void createMesh() override;

private:
    uint32_t vertexSize_ = 32;
    uint32_t indexSize_  = 32;

    float radius_ = 1.f;

public: // accessor
    float getRadius() const {
        return radius_;
    }
    void setRadius(float _radius) {
        radius_ = _radius;
    }

    uint32_t getVertexSize() const {
        return vertexSize_;
    }
    void setVertexSize(uint32_t _vertexSize_) {
        vertexSize_ = _vertexSize_;
    }
};

class Ring
    : public IPrimitive {
public:
    Ring() : IPrimitive(PrimitiveType::Ring) {}
    ~Ring() override {
        mesh_.reset();
    }
    void Initialize() override {
        vertexSize_ = 4 * division_ * division_;
        indexSize_  = 4 * division_ * division_;
        // mesh Init
        mesh_ = std::make_shared<TextureMesh>();
        mesh_->setName("Ring");
        mesh_->Initialize(vertexSize_, indexSize_);
        // create mesh
        createMesh();
    }
    void createMesh() override;

private:
    uint32_t division_   = 16;
    uint32_t vertexSize_ = 0;
    uint32_t indexSize_  = 0;

    float radius_      = 1.f;
    float innerRadius_ = 0.5f;
    float uv_          = 1.f;
    float uvInner_     = 0.5f;
};

class Sphere
    : public IPrimitive {
public:
    Sphere() : IPrimitive(PrimitiveType::Sphere) {}
    ~Sphere() override {
        mesh_.reset();
    }
    void Initialize() override {
        vertexSize_ = 4 * division_ * division_;
        indexSize_  = 4 * division_ * division_;
        // mesh Init
        mesh_ = std::make_shared<TextureMesh>();
        mesh_->setName("Sphere");
        mesh_->Initialize(vertexSize_, indexSize_);
        // create mesh
        createMesh();
    }
    void createMesh() override;

private:
    uint32_t division_   = 16;
    uint32_t vertexSize_ = 0;
    uint32_t indexSize_  = 0;

    Vec3f radius_ = {1.f, 1.f, 1.f};
    Vec3f uv_     = {1.f, 1.f, 1.f};

public: // accessor
    const Vec3f& getRadius() const {
        return radius_;
    }
    void setRadius(const Vec3f& _radius) {
        radius_ = _radius;
    }

    const Vec3f& getUV() const {
        return uv_;
    }
    void setUV(const Vec3f& _uv) {
        uv_ = _uv;
    }

    uint32_t getDivision() const {
        return division_;
    }
    void setDivision(uint32_t _division) {
        division_ = _division;
    }
};

class Torus
    : public IPrimitive {
public:
    Torus() : IPrimitive(PrimitiveType::Torus) {}
    ~Torus() override {
        mesh_.reset();
    }
    void Initialize() override {
        vertexSize_ = 4 * division_ * division_;
        indexSize_  = 4 * division_ * division_;
        // mesh Init
        mesh_ = std::make_shared<TextureMesh>();
        mesh_->setName("Torus");
        mesh_->Initialize(vertexSize_, indexSize_);
        // create mesh
        createMesh();
    }
    void createMesh() override;

private:
    uint32_t division_ = 16;
    uint32_t vertexSize_;
    uint32_t indexSize_;

    float radius_ = 1.f;
    float uv_     = 1.f;
};

class Cylinder
    : public IPrimitive {
public:
    Cylinder() : IPrimitive(PrimitiveType::Cylinder) {}
    ~Cylinder() override {
        mesh_.reset();
    }

    void Initialize() override {
        vertexSize_ = 4 * division_ * division_;
        indexSize_  = 4 * division_ * division_;
        // mesh Init
        mesh_ = std::make_shared<TextureMesh>();
        mesh_->setName("Cylinder");
        mesh_->Initialize(vertexSize_, indexSize_);
        // create mesh
        createMesh();
    }

    void createMesh() override;

private:
    uint32_t division_ = 16;
    uint32_t vertexSize_;
    uint32_t indexSize_;

    float radius_ = 1.f;
    float uv_     = 1.f;

    float height_ = 1.f;
};

class Cone
    : public IPrimitive {
public:
    Cone() : IPrimitive(PrimitiveType::Cone) {}
    ~Cone() override {
        mesh_.reset();
    }
    void Initialize() override {
        vertexSize_ = 4 * division_ * division_;
        indexSize_  = 4 * division_ * division_;
        // mesh Init
        mesh_ = std::make_shared<TextureMesh>();
        mesh_->setName("Cone");
        mesh_->Initialize(vertexSize_, indexSize_);
        // create mesh
        createMesh();
    }

    void createMesh() override;

private:
    uint32_t division_ = 16;
    uint32_t vertexSize_;
    uint32_t indexSize_;
    float radius_ = 1.f;
    float uv_     = 1.f;
    float height_ = 1.f;
};

class Triangle
    : public IPrimitive {
public:
    Triangle() : IPrimitive(PrimitiveType::Triangel) {}
    ~Triangle() override {
        mesh_.reset();
    }
    void Initialize() override {
        // mesh Init
        mesh_ = std::make_shared<TextureMesh>();
        mesh_->setName("Triangle");
        mesh_->Initialize(3, 3);
        // create mesh
        createMesh();
    }
    void createMesh() override;

private:
    Vec3f vertex_[3] = {
        {0.f, 0.f, 0.f},
        {1.f, 0.f, 0.f},
        {0.f, 1.f, 0.f},
    };
    Vec3f normal_ = {0.f, 0.f, 1.f};
    Vec2f uv_     = {0.f, 0.f};
};

class Box
    : public IPrimitive {
public:
    Box() : IPrimitive(PrimitiveType::Box) {}
    ~Box() override {
        mesh_.reset();
    }
    void Initialize() override {
        // mesh Init
        mesh_ = std::make_shared<TextureMesh>();
        mesh_->setName("Box");
        mesh_->Initialize(vertexSize, indexSize);
        // create mesh
        createMesh();
    }
    void createMesh() override;

private:
    const uint32_t vertexSize = 8;
    const uint32_t indexSize  = 24;

    Vec3f size_ = {1.f, 1.f, 1.f};

    Vec3f uv_ = {1.f, 1.f, 1.f};
};

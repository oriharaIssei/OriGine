#pragma once

/// stl
#include <memory>

/// engine
// dreictX12
#include "directX12/Mesh.h"
// component
#include "component/renderer/MeshRenderer.h"

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
    Triangle, // 三角形
    Box // 立方体
};

/// <summary>
/// 形状の基底クラス
/// </summary>
class IPrimitive {
public:
    IPrimitive(PrimitiveType _type) : type_(_type) {}
    virtual ~IPrimitive() {}

    virtual void createMesh(TextureMesh* _mesh) = 0;

protected:
private:
    PrimitiveType type_;

public: // accessor
    PrimitiveType getType() const { return type_; }
};

/// <summary>
/// Plane(面)のPrimitiveクラス
/// </summary>
class Plane
    : public IPrimitive {
public:
    Plane() : IPrimitive(PrimitiveType::Plane) {}
    ~Plane() override {}

    void createMesh(TextureMesh* _mesh) override;

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

/// <summary>
/// Circle(円)のPrimitiveクラス
/// </summary>
class Circle
    : public IPrimitive {
public:
    Circle() : IPrimitive(PrimitiveType::Circle) {}
    ~Circle() override {}

    void createMesh(TextureMesh* _mesh) override;

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
    uint32_t getIndexSize() const {
        return indexSize_;
    }
    void setIndexSize(uint32_t _indexSize_) {
        indexSize_ = _indexSize_;
    }
};

/// <summary>
/// Ring(環)のPrimitiveクラス
/// </summary>
class Ring
    : public IPrimitive {
public:
    Ring() : IPrimitive(PrimitiveType::Ring) {}
    ~Ring() override {}

    void createMesh(TextureMesh* _mesh) override;

private:
    uint32_t division_   = 16;
    uint32_t vertexSize_ = 0;
    uint32_t indexSize_  = 0;

    float outerRadius_ = 1.f;
    float innerRadius_ = 0.5f;

public:
    float getOuterRadius() const {
        return outerRadius_;
    }
    void setOuterRadius(float _outerRadius) {
        outerRadius_ = _outerRadius;
    }
    float getInnerRadius() const {
        return innerRadius_;
    }
    void setInnerRadius(float _innerRadius) {
        innerRadius_ = _innerRadius;
    }
    uint32_t getDivision() const {
        return division_;
    }
    void setDivision(uint32_t _division) {
        division_ = _division;
    }
};

/// <summary>
/// Sphere(球)のPrimitiveクラス
/// </summary>
// class Sphere
//     : public IPrimitive {
// public:
//     Sphere() : IPrimitive(PrimitiveType::Sphere) {}
//     ~Sphere() override {}
//
//     void createMesh(TextureMesh* _mesh) override;
//
// private:
//     uint32_t division_ = 16;
//
//     Vec3f radius_ = {1.f, 1.f, 1.f};
//     Vec3f uv_     = {1.f, 1.f, 1.f};
//
// public: // accessor
//     const Vec3f& getRadius() const {
//         return radius_;
//     }
//     void setRadius(const Vec3f& _radius) {
//         radius_ = _radius;
//     }
//
//     const Vec3f& getUV() const {
//         return uv_;
//     }
//     void setUV(const Vec3f& _uv) {
//         uv_ = _uv;
//     }
//
//     uint32_t getDivision() const {
//         return division_;
//     }
//     void setDivision(uint32_t _division) {
//         division_ = _division;
//     }
//
//     uint32_t getVertexSize() const {
//         return 4 * division_ * division_;
//     }
//     uint32_t getIndexSize() const {
//         return 4 * division_ * division_;
//     }
// };

/// <summary>
/// Torus(トーラス)のPrimitiveクラス
/// </summary>

/*class Torus
    : public IPrimitive {
public:
    Torus() : IPrimitive(PrimitiveType::Torus) {}
    ~Torus() override {
        mesh_.reset();
    }
    void Initialize() override {
        vertexSize_ = 4 * division_ * division_;
        indexSize_  = 4 * division_ * division_;
        // _mesh Init
        mesh_ = std::make_shared<TextureMesh>();
        mesh_->setName("Torus");
        mesh_->Initialize(vertexSize_, indexSize_);
        // create _mesh
        createMesh(TextureMesh* _mesh);
    }
    void createMesh(TextureMesh* _mesh) override;

private:
    uint32_t division_ = 16;
    uint32_t vertexSize_;
    uint32_t indexSize_;

    float radius_ = 1.f;
    float uv_     = 1.f;
};
*/

/// <summary>
/// Cylinder(円柱)のPrimitiveクラス
/// </summary>

/*
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
        // _mesh Init
        mesh_ = std::make_shared<TextureMesh>();
        mesh_->setName("Cylinder");
        mesh_->Initialize(vertexSize_, indexSize_);
        // create _mesh
        createMesh(TextureMesh* _mesh);
    }

    void createMesh(TextureMesh* _mesh) override;

private:
    uint32_t division_ = 16;
    uint32_t vertexSize_;
    uint32_t indexSize_;

    float radius_ = 1.f;
    float uv_     = 1.f;

    float height_ = 1.f;
};
*/

/// <summary>
/// Cone(円錐)のPrimitiveクラス
/// </summary>
/*
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
        // _mesh Init
        mesh_ = std::make_shared<TextureMesh>();
        mesh_->setName("Cone");
        mesh_->Initialize(vertexSize_, indexSize_);
        // create _mesh
        createMesh(TextureMesh* _mesh);
    }

    void createMesh(TextureMesh* _mesh) override;

private:
    uint32_t division_ = 16;
    uint32_t vertexSize_;
    uint32_t indexSize_;
    float radius_ = 1.f;
    float uv_     = 1.f;
    float height_ = 1.f;
};
*/

/// <summary>
/// Triangle(三角形)のPrimitiveクラス
/// </summary>
/*
class Triangle
    : public IPrimitive {
public:
    Triangle() : IPrimitive(PrimitiveType::Triangle) {}
    ~Triangle() override {
        mesh_.reset();
    }
    void Initialize() override {
        // _mesh Init
        mesh_ = std::make_shared<TextureMesh>();
        mesh_->setName("Triangle");
        mesh_->Initialize(3, 3);
        // create _mesh
        createMesh(TextureMesh* _mesh);
    }
    void createMesh(TextureMesh* _mesh) override;

private:
    Vec3f vertex_[3] = {
        {1.f, 0.f, 0.f},
        {0.f, 1.f, 0.f},
        {-1.f, 0.f, 0.f},
    };
    Vec3f normal_ = {0.f, 0.f, 1.f};
    Vec2f uv_     = {0.f, 0.f};
};
*/

/// <summary>
/// Box(立方体)のPrimitiveクラス
/// </summary>
/*
class Box
    : public IPrimitive {
public:
    Box() : IPrimitive(PrimitiveType::Box) {}
    ~Box() override {
        mesh_.reset();
    }
    void Initialize() override {
        // _mesh Init
        mesh_ = std::make_shared<TextureMesh>();
        mesh_->setName("Box");
        mesh_->Initialize(vertexSize, indexSize);
        // create _mesh
        createMesh(TextureMesh* _mesh);
    }
    void createMesh(TextureMesh* _mesh) override;

private:
    const uint32_t vertexSize = 8;
    const uint32_t indexSize  = 24;

    Vec3f size_ = {1.f, 1.f, 1.f};

    Vec3f uv_ = {1.f, 1.f, 1.f};
};
*/

template <typename T>
concept IsPrimitive = std::derived_from<T, IPrimitive>;

template <IsPrimitive PrimType>
class PrimitiveMeshRenderer
    : public MeshRenderer<TextureMesh, TextureVertexData> {
public:
    PrimitiveMeshRenderer() : MeshRenderer() {}
    PrimitiveMeshRenderer(const std::vector<TextureMesh>& _meshGroup) : MeshRenderer(_meshGroup) {}
    PrimitiveMeshRenderer(const std::shared_ptr<std::vector<TextureMesh>>& _meshGroup) : MeshRenderer(_meshGroup) {}
    ~PrimitiveMeshRenderer() override {
    }

    void Initialize(GameEntity* _entity) = 0;

    void Finalize() override {
        for (auto& mesh : *meshGroup_) {
            mesh.Finalize();
        }
        meshGroup_.reset();
        transformBuff_.Finalize();
        materialBuff_.Finalize();
    }

    using PrimitiveType = PrimType;

protected:
    virtual void createMesh(TextureMesh* _mesh) {
        primitive_.createMesh(_mesh);
    }

protected:
    PrimType primitive_;
    IConstantBuffer<Transform> transformBuff_;
    IConstantBuffer<Material> materialBuff_;

    std::string textureDirectory_;
    std::string textureFileName_;
    uint32_t textureIndex_ = 0;

public:
    const PrimType& getPrimitive() const {
        return primitive_;
    }
    PrimType& getPrimitive() {
        return primitive_;
    }

    const Transform& getTransform() const {
        return transformBuff_.openData_;
    }
    void setTransform(const Transform& _transform) {
        transformBuff_.openData_ = _transform;
    }
    const Material& getMaterial() const {
        return materialBuff_.openData_;
    }
    void setMaterial(const Material& _material) {
        materialBuff_.openData_ = _material;
    }

    const IConstantBuffer<Transform>& getTransformBuff() const {
        return transformBuff_;
    }
    IConstantBuffer<Transform>& getTransformBuff() {
        return transformBuff_;
    }
    const IConstantBuffer<Material>& getMaterialBuff() const {
        return materialBuff_;
    }
    IConstantBuffer<Material>& getMaterialBuff() {
        return materialBuff_;
    }

    const std::string& getTextureDirectory() const {
        return textureDirectory_;
    }
    const std::string& getTextureFileName() const {
        return textureFileName_;
    }

    uint32_t getTextureIndex() const {
        return textureIndex_;
    }
    void loadTexture(const std::string& _directory, const std::string& _filename) {
        textureDirectory_ = _directory;
        textureFileName_  = _filename;
        textureIndex_     = TextureManager::LoadTexture(_directory + "/" + _filename);
    }
};

class PlaneRenderer
    : public PrimitiveMeshRenderer<Plane> {
    friend void to_json(nlohmann::json& j, const PlaneRenderer& r);
    friend void from_json(const nlohmann::json& j, PlaneRenderer& r);

public:
    PlaneRenderer() : PrimitiveMeshRenderer() {}
    PlaneRenderer(const std::vector<TextureMesh>& _meshGroup) : PrimitiveMeshRenderer(_meshGroup) {}
    PlaneRenderer(const std::shared_ptr<std::vector<TextureMesh>>& _meshGroup) : PrimitiveMeshRenderer(_meshGroup) {}
    ~PlaneRenderer() override {}

    void Initialize(GameEntity* _hostEntity) override {
        _hostEntity;
        // _mesh Init
        if (!meshGroup_->empty()) {
            meshGroup_->clear();
        }

        meshGroup_->emplace_back(MeshType());
        auto& mesh = meshGroup_->back();
        mesh.Initialize(4, 6);

        transformBuff_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
        materialBuff_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());

        // create _mesh
        createMesh(&mesh);

        // loadTexture
        if (!textureDirectory_.empty() && !textureFileName_.empty()) {
            textureIndex_ = TextureManager::LoadTexture(textureDirectory_ + "/" + textureFileName_);
        }
    }

    bool Edit() override;
};

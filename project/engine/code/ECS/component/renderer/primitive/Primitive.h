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
#include <stdint.h>
#include <Vector2.h>
#include <Vector3.h>
#include <Vector4.h>

enum class PrimitiveType : int32_t {
    Plane, // 面
    Ring, // 環(真ん中が空洞)
    // Circle, // 円
    Box, // 立方体
    Sphere, // 球
    //  Torus, // トーラス
    //  Cylinder, // 円柱
    //  Cone // 円錐

    Count
};
const char* PrimitiveTypeToString(PrimitiveType _type);

/// <summary>
/// 形状の基底クラス
/// </summary>
class IPrimitive {
public:
    IPrimitive(PrimitiveType _type) : type_(_type) {}
    virtual ~IPrimitive() {}

    virtual void createMesh(TextureMesh* _mesh) = 0;

protected:
    int32_t vertexSize_ = 0; // 頂点数
    int32_t indexSize_  = 0; // インデックス数
private:
    PrimitiveType type_;

public: // accessor
    PrimitiveType getType() const { return type_; }

    int32_t getVertexSize() const {
        return vertexSize_;
    }
    void setVertexSize(int32_t _size) {
        vertexSize_ = _size;
    }
    int32_t getIndexSize() const {
        return indexSize_;
    }
    void setIndexSize(int32_t _size) {
        indexSize_ = _size;
    }
};

namespace Primitive {

#pragma region "PrimitiveData"

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
/// Box(立方体)のPrimitiveクラス
/// </summary>
class Box
    : public IPrimitive {
public:
    Box() : IPrimitive(PrimitiveType::Box) {}
    ~Box() override {}
    void createMesh(TextureMesh* _mesh) override;

private:
    int32_t vertexSize_ = 8; // 立方体の頂点数
    int32_t indexSize_  = 36; // 立方体のインデックス数
    Vec3f size_         = {1.0f, 1.0f, 1.0f}; // サイズ
public:
    const Vec3f& getSize() const {
        return size_;
    }
    void setSize(const Vec3f& _size) {
        size_ = _size;
    }
};

/// <summary>
/// Sphere(球)のPrimitiveクラス
/// </summary>
class Sphere
    : public IPrimitive {
public:
    Sphere() : IPrimitive(PrimitiveType::Sphere) {}
    ~Sphere() override {}
    void createMesh(TextureMesh* _mesh) override;

private:
    float radius_               = 1.0f; // 半径
    uint32_t divisionLatitude_  = 8; // 緯度分割数
    uint32_t divisionLongitude_ = 16; // 経度分割数
public:
    float getRadius() const {
        return radius_;
    }
    void setRadius(float _radius) {
        radius_ = _radius;
    }
    uint32_t getDivisionLatitude() const {
        return divisionLatitude_;
    }
    void setDivisionLatitude(uint32_t _division) {
        divisionLatitude_ = _division;
    }
    uint32_t getDivisionLongitude() const {
        return divisionLongitude_;
    }
    void setDivisionLongitude(uint32_t _division) {
        divisionLongitude_ = _division;
    }
};

}

#pragma endregion

template <typename T>
concept IsPrimitive = std::derived_from<T, IPrimitive>;

/// <summary>
/// PrimitiveRendererをポリモーフィズムで扱うための基底クラス
/// </summary>
class PrimitiveMeshRendererBase
    : public MeshRenderer<TextureMesh, TextureVertexData> {
public:
    PrimitiveMeshRendererBase() : MeshRenderer() {}
    PrimitiveMeshRendererBase(const std::vector<TextureMesh>& _meshGroup) : MeshRenderer(_meshGroup) {}
    PrimitiveMeshRendererBase(const std::shared_ptr<std::vector<TextureMesh>>& _meshGroup) : MeshRenderer(_meshGroup) {}

    virtual ~PrimitiveMeshRendererBase()                                                                    = default;
    virtual void Initialize(GameEntity* _entity)                                                            = 0;
    virtual void Finalize()                                                                                 = 0;
    virtual void Edit(Scene* _scene, GameEntity* _entity, [[maybe_unused]] const std::string& _parentLabel) = 0;

    virtual void createMesh(TextureMesh* _mesh) = 0;

protected:
    IConstantBuffer<Transform> transformBuff_;
    int32_t materialIndex_ = -1;
    SimpleConstantBuffer<Material> materialBuff_;

    std::string textureDirectory_;
    std::string textureFileName_;
    uint32_t textureIndex_ = 0;

public:
    Transform& getTransform() {
        return transformBuff_.openData_;
    }
    void setTransform(const Transform& _transform) {
        transformBuff_.openData_ = _transform;
    }
    int32_t getMaterialIndex() const {
        return materialIndex_;
    }
    void setMaterialIndex(int32_t _index) {
        materialIndex_ = _index;
    }

    const IConstantBuffer<Transform>& getTransformBuff() const {
        return transformBuff_;
    }
    IConstantBuffer<Transform>& getTransformBuff() {
        return transformBuff_;
    }
    const SimpleConstantBuffer<Material>& getMaterialBuff() const {
        return materialBuff_;
    }
    SimpleConstantBuffer<Material>& getMaterialBuff() {
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

std::shared_ptr<PrimitiveMeshRendererBase> CreatePrimitiveRenderer(PrimitiveType _type);

template <IsPrimitive PrimType>
class PrimitiveMeshRenderer
    : public PrimitiveMeshRendererBase {
public:
    PrimitiveMeshRenderer() : PrimitiveMeshRendererBase() {}
    PrimitiveMeshRenderer(const std::vector<TextureMesh>& _meshGroup) : PrimitiveMeshRendererBase(_meshGroup) {}
    PrimitiveMeshRenderer(const std::shared_ptr<std::vector<TextureMesh>>& _meshGroup) : PrimitiveMeshRendererBase(_meshGroup) {}

    ~PrimitiveMeshRenderer() override {}

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

    virtual void createMesh(TextureMesh* _mesh) {
        primitive_.createMesh(_mesh);
    }

protected:
    PrimType primitive_;

public:
    const PrimType& getPrimitive() const {
        return primitive_;
    }
    PrimType& getPrimitive() {
        return primitive_;
    }
};

#pragma region "PrimitiveRenderer"

class PlaneRenderer
    : public PrimitiveMeshRenderer<Primitive::Plane> {
    friend void to_json(nlohmann::json& j, const PlaneRenderer& r);
    friend void from_json(const nlohmann::json& j, PlaneRenderer& r);

public:
    PlaneRenderer() : PrimitiveMeshRenderer() {}
    PlaneRenderer(const std::vector<TextureMesh>& _meshGroup) : PrimitiveMeshRenderer(_meshGroup) {}
    PlaneRenderer(const std::shared_ptr<std::vector<TextureMesh>>& _meshGroup) : PrimitiveMeshRenderer(_meshGroup) {}
    ~PlaneRenderer() override {}

    void Initialize(GameEntity* _hostEntity) override;

    void Edit(Scene* _scene, GameEntity* _entity, [[maybe_unused]] const std::string& _parentLabel) override;
};

class RingRenderer
    : public PrimitiveMeshRenderer<Primitive::Ring> {
    friend void to_json(nlohmann::json& j, const RingRenderer& r);
    friend void from_json(const nlohmann::json& j, RingRenderer& r);

public:
    RingRenderer() : PrimitiveMeshRenderer() {}
    RingRenderer(const std::vector<TextureMesh>& _meshGroup) : PrimitiveMeshRenderer(_meshGroup) {}
    RingRenderer(const std::shared_ptr<std::vector<TextureMesh>>& _meshGroup) : PrimitiveMeshRenderer(_meshGroup) {}
    ~RingRenderer() override {}

    void Initialize(GameEntity* _hostEntity) override;

    void Edit(Scene* _scene, GameEntity* _entity, [[maybe_unused]] const std::string& _parentLabel) override;
};

class BoxRenderer
    : public PrimitiveMeshRenderer<Primitive::Box> {
    friend void to_json(nlohmann::json& j, const BoxRenderer& r);
    friend void from_json(const nlohmann::json& j, BoxRenderer& r);

public:
    BoxRenderer() : PrimitiveMeshRenderer() {}
    BoxRenderer(const std::vector<TextureMesh>& _meshGroup) : PrimitiveMeshRenderer(_meshGroup) {}
    BoxRenderer(const std::shared_ptr<std::vector<TextureMesh>>& _meshGroup) : PrimitiveMeshRenderer(_meshGroup) {}
    ~BoxRenderer() override {}

    void Initialize(GameEntity* _hostEntity) override;

    void Edit(Scene* _scene, GameEntity* _entity, [[maybe_unused]] const std::string& _parentLabel) override;
};

class SphereRenderer
    : public PrimitiveMeshRenderer<Primitive::Sphere> {
    friend void to_json(nlohmann::json& j, const SphereRenderer& r);
    friend void from_json(const nlohmann::json& j, SphereRenderer& r);

public:
    SphereRenderer() : PrimitiveMeshRenderer() {}
    SphereRenderer(const std::vector<TextureMesh>& _meshGroup) : PrimitiveMeshRenderer(_meshGroup) {}
    SphereRenderer(const std::shared_ptr<std::vector<TextureMesh>>& _meshGroup) : PrimitiveMeshRenderer(_meshGroup) {}

    ~SphereRenderer() override {}

    void Initialize(GameEntity* _hostEntity) override;
    void Edit(Scene* _scene, GameEntity* _entity, [[maybe_unused]] const std::string& _parentLabel) override;
};

#pragma endregion

#pragma once

/// stl
#include <concepts>
#include <vector>

/// engine
#include "Engine.h"
// directX12
#include "d3d12.h"
#include "directX12/DxDevice.h"
#include "directX12/DxResource.h"

/// math
#include <Vector2.h>
#include <Vector3.h>
#include <Vector4.h>

/// <summary>
/// 1頂点を表すデータ(テクスチャあり)
/// </summary>
struct TextureVertexData {
    Vec4f pos;
    Vec2f texCoord;
    Vec3f normal;
    TextureVertexData* operator=(const TextureVertexData& vert) {
        this->pos      = vert.pos;
        this->texCoord = vert.texCoord;
        this->normal   = vert.normal;
        return this;
    }

    bool operator==(const TextureVertexData& vert) {
        if (this->pos != vert.pos) {
            return false;
        }
        if (this->texCoord != vert.texCoord) {
            return false;
        }
        if (this->normal != vert.normal) {
            return false;
        }
        return true;
    }
};
/// <summary>
/// 1頂点を表すデータ(テクスチャなし)
/// </summary>
struct PrimitiveVertexData {
    Vec4f pos;
    Vec3f normal;
    PrimitiveVertexData(const TextureVertexData& vert) {
        this->pos    = vert.pos;
        this->normal = vert.normal;
    }
    PrimitiveVertexData* operator=(const PrimitiveVertexData& vert) {
        this->pos    = vert.pos;
        this->normal = vert.normal;
        return this;
    }
    PrimitiveVertexData* operator=(const TextureVertexData& vert) {
        this->pos    = vert.pos;
        this->normal = vert.normal;
        return this;
    }
};

struct ColorVertexData {
    Vec4f pos = {0.f, 0.f, 0.f, 1.f};
    Vec4f color;

    ColorVertexData* operator=(const ColorVertexData& vert) {
        this->pos   = vert.pos;
        this->color = vert.color;
        return this;
    }
};

/// <summary>
/// メッシュデータ
/// </summary>
template <typename VertexDataType>
class Mesh {
public:
    using VertexType = VertexDataType;
    using IndexType  = uint32_t;

    Mesh() {};
    virtual ~Mesh() {};

    /// <summary>
    /// VertexDataを設定後に実行
    /// </summary>
    /// <param name="vertexSize">総頂点数</param>
    void Initialize(
        UINT _vertexCapacity,
        UINT _indexCapacity) {

        this->vertexSize_ = _vertexCapacity;
        this->indexSize_  = _indexCapacity;

        if (this->vertexSize_ != 0) {
            vertexes_.resize(this->vertexSize_);

            UINT vertDataSize = sizeof(VertexDataType);

            vertBuff_.CreateBufferResource(Engine::getInstance()->getDxDevice()->getDevice(), vertDataSize * this->vertexSize_);
            vbView_.BufferLocation = vertBuff_.getResource()->GetGPUVirtualAddress();
            vbView_.SizeInBytes    = vertDataSize * this->vertexSize_;
            vbView_.StrideInBytes  = vertDataSize;
            vertBuff_.getResource()->Map(0, nullptr, reinterpret_cast<void**>(&vertData_));
        }

        if (this->indexSize_ != 0) {
            indexes_.resize(this->indexSize_);

            UINT indexDataSize = sizeof(uint32_t);
            indexBuff_.CreateBufferResource(Engine::getInstance()->getDxDevice()->getDevice(), indexDataSize * this->indexSize_);
            ibView_.BufferLocation = indexBuff_.getResource()->GetGPUVirtualAddress();
            ibView_.SizeInBytes    = indexDataSize * this->indexSize_;
            ibView_.Format         = DXGI_FORMAT_R32_UINT;
            indexBuff_.getResource()->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));
        }
    }

    void Finalize() {
        vertBuff_.Finalize();
        indexBuff_.Finalize();
    }

public:
    void TransferData() {
        if (this->vertexSize_ != 0) {
            size_t validSize = vertexes_.size();
            size_t totalSize = this->vertexSize_;

            // 有効データをコピー
            if (validSize > 0) {
                memcpy(vertData_, vertexes_.data(), sizeof(VertexDataType) * validSize);
            }
            // 未使用領域をゼロクリア
            if (validSize < totalSize) {
                memset(
                    reinterpret_cast<char*>(vertData_) + sizeof(VertexDataType) * validSize,
                    0,
                    sizeof(VertexDataType) * (totalSize - validSize));
            }
        }
        if (this->indexSize_ != 0) {
            size_t validSize = indexes_.size();
            size_t totalSize = this->indexSize_;

            // 有効データをコピー
            if (validSize > 0) {
                memcpy(indexData_, indexes_.data(), sizeof(uint32_t) * validSize);
            }
            // 未使用領域をゼロクリア
            if (validSize < totalSize) {
                memset(
                    reinterpret_cast<char*>(indexData_) + sizeof(uint32_t) * validSize,
                    0,
                    sizeof(uint32_t) * (totalSize - validSize));
            }
        }
        if (this->indexSize_ != 0) {
            memcpy(indexData_, indexes_.data(), sizeof(uint32_t) * this->indexSize_);
        }
    }

public:
    // data
    std::vector<VertexDataType> vertexes_;
    std::vector<uint32_t> indexes_;

protected:
    VertexDataType* vertData_ = nullptr;
    uint32_t* indexData_      = nullptr;

    uint32_t vertexSize_ = 0;
    uint32_t indexSize_  = 0;

    // buffer
    DxResource vertBuff_;
    DxResource indexBuff_;

    // view
    D3D12_INDEX_BUFFER_VIEW ibView_{};
    D3D12_VERTEX_BUFFER_VIEW vbView_{};

private:
    std::string name_ = "UNKNOWN";

public:
    void setName(const std::string& _name) {
        name_ = _name;
    }
    const std::string& getName() const {
        return name_;
    }

    void setVertexData(const std::vector<VertexDataType>& _data) {
        vertexes_ = _data;
    }
    void setIndexData(const std::vector<uint32_t>& _data) {
        indexes_ = _data;
    }

    void copyVertexData(const VertexDataType* _data, uint32_t _size) {
        if (vertexSize_ < _size) {
            assert("vertexSize < _size");
        }

        if (_size > vertexes_.size()) {
            vertexes_.resize(_size);
        }
        memcpy(vertexes_.data(), _data, sizeof(VertexDataType) * _size);
    }
    void copyIndexData(const uint32_t* _data, uint32_t _size) {
        if (indexSize_ < _size) {
            assert("indexCapacity_ < _size");
        }
        if (_size > indexes_.size()) {
            indexes_.resize(_size);
        }
        memcpy(indexes_.data(), _data, sizeof(uint32_t) * _size);
    }

    DxResource& getVertexBuffer() {
        return vertBuff_;
    }
    DxResource& getIndexBuffer() {
        return indexBuff_;
    }

    int32_t getVertexCapacity() const {
        return int32_t(vertexSize_) - int32_t(vertexes_.size());
    }
    int32_t getIndexCapacity() const {
        return int32_t(indexSize_) - int32_t(indexes_.size());
    }

    uint32_t getVertexSize() const {
        return vertexSize_;
    }
    uint32_t getIndexSize() const {
        return indexSize_;
    }
    void setVertexSize(uint32_t _size) {
        vertexSize_ = _size;
        vertexes_.resize(vertexSize_);
    }
    void setIndexSize(uint32_t _size) {
        indexSize_ = _size;
        indexes_.resize(indexSize_);
    }

    const D3D12_VERTEX_BUFFER_VIEW& getVertexBufferView() const {
        return vbView_;
    }
    const D3D12_INDEX_BUFFER_VIEW& getIndexBufferView() const {
        return ibView_;
    }
    const D3D12_VERTEX_BUFFER_VIEW& getVBView() const {
        return vbView_;
    }
    const D3D12_INDEX_BUFFER_VIEW& getIBView() const {
        return ibView_;
    }
};

using TextureMesh   = Mesh<TextureVertexData>;
using PrimitiveMesh = Mesh<PrimitiveVertexData>;

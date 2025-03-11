#pragma once

/// stl
#include <concepts>
#include <vector>

/// engine
#include "Engine.h"
// directX12
#include "d3d12.h"
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

/// <summary>
/// メッシュデータ
/// </summary>
template <typename VertexDataType>
class Mesh {
public:
    Mesh() {};
    virtual ~Mesh() {};

    /// <summary>
    /// VertexDataを設定後に実行
    /// </summary>
    /// <param name="vertexSize">総頂点数</param>
    void Init(UINT _vertexSize, UINT _indexSize) {
        this->vertexSize = static_cast<uint32_t>(_vertexSize);
        this->indexSize  = static_cast<uint32_t>(_indexSize);

        if (this->vertexSize != 0) {
            UINT vertDataSize = sizeof(VertexDataType);

            vertBuff.CreateBufferResource(Engine::getInstance()->getDxDevice()->getDevice(), vertDataSize * this->vertexSize);
            vbView.BufferLocation = vertBuff.getResource()->GetGPUVirtualAddress();
            vbView.SizeInBytes    = vertDataSize * this->vertexSize;
            vbView.StrideInBytes  = vertDataSize;
            vertBuff.getResource()->Map(0, nullptr, reinterpret_cast<void**>(&vertData));
        }

        if (this->indexSize != 0) {
            UINT indexDataSize = sizeof(uint32_t);
            indexBuff.CreateBufferResource(Engine::getInstance()->getDxDevice()->getDevice(), indexDataSize * this->indexSize);
            ibView.BufferLocation = indexBuff.getResource()->GetGPUVirtualAddress();
            ibView.SizeInBytes    = indexDataSize * this->indexSize;
            ibView.Format         = DXGI_FORMAT_R32_UINT;
            indexBuff.getResource()->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
        }
    }

    void Finalize() {
        vertBuff.Finalize();
        indexBuff.Finalize();
    }

public:
    // data
    VertexDataType* vertData = nullptr;
    uint32_t* indexData      = nullptr;

    uint32_t vertexSize = 0;
    uint32_t indexSize  = 0;

    // buffer
    DxResource vertBuff;
    DxResource indexBuff;

    // view
    D3D12_INDEX_BUFFER_VIEW ibView{};
    D3D12_VERTEX_BUFFER_VIEW vbView{};

private:
    std::string name_ = "UNKNOWN";

public:
    void setName(const std::string& _name) {
        name_ = _name;
    }
    const std::string& getName() const {
        return name_;
    }
};

using TextureMesh   = Mesh<TextureVertexData>;
using PrimitiveMesh = Mesh<PrimitiveVertexData>;


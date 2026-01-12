#pragma once

/// stl
#include <concepts>
#include <vector>

/// engine
#include "Engine.h"
// directX12
#include "../DxDevice.h"
#include "../DxResource.h"
#include "d3d12.h"
#include "DefaultVertexData.h"

namespace OriGine {

/// <summary>
/// 頂点バッファとインデックスバッファをペアで管理し、GPU への転送と描画用ビューの保持を行うクラス.
/// </summary>
/// <typeparam name="VertexDataType">頂点構造体の型</typeparam>
template <typename VertexDataType>
class Mesh {
public:
    using VertexType = VertexDataType;
    using IndexType  = uint32_t;

    Mesh() {};
    virtual ~Mesh() {};

    /// <summary>
    /// バッファのキャパシティを指定して初期化し、リソースを生成する.
    /// </summary>
    /// <param name="_vertexCapacity">最大頂点数</param>
    /// <param name="_indexCapacity">最大インデックス数</param>
    void Initialize(
        UINT _vertexCapacity,
        UINT _indexCapacity);

    /// <summary>
    /// 生成したリソースを解放する.
    /// </summary>
    void Finalize();

    /// <summary>
    /// 現在 CPU 上に保持しているデータ (vertexes_, indexes_) を GPU 用バッファに転送する.
    /// </summary>
    void TransferData();

    /// <summary>
    /// 任意の頂点配列を内部の CPU ベクトルにコピーする.
    /// </summary>
    /// <param name="_data">コピー元ポインタ</param>
    /// <param name="_size">頂点数</param>
    void copyVertexData(const VertexDataType* _data, uint32_t _size);

    /// <summary>
    /// 任意のインデックス配列を内部の CPU ベクトルにコピーする.
    /// </summary>
    /// <param name="_data">コピー元ポインタ</param>
    /// <param name="_size">インデックス数</param>
    void copyIndexData(const uint32_t* _data, uint32_t _size);

public:
    /// <summary>CPU側で保持する頂点データの配列</summary>
    std::vector<VertexDataType> vertexes_;
    /// <summary>CPU側で保持するインデックスデータの配列</summary>
    std::vector<uint32_t> indexes_;

protected:
    /// <summary>マッピングされた頂点バッファへのポインタ</summary>
    VertexDataType* vertData_ = nullptr;
    /// <summary>マッピングされたインデックスバッファへのポインタ</summary>
    uint32_t* indexData_ = nullptr;

    /// <summary>バッファに書き込まれた有効な頂点数</summary>
    uint32_t vertexSize_ = 0;
    /// <summary>バッファに書き込まれた有効なインデックス数</summary>
    uint32_t indexSize_ = 0;

    /// <summary>頂点リソース</summary>
    DxResource vertBuff_;
    /// <summary>インデックスリソース</summary>
    DxResource indexBuff_;

    /// <summary>インデックスバッファビュー (ID3D12GraphicsCommandList::IASetIndexBuffer 用)</summary>
    D3D12_INDEX_BUFFER_VIEW ibView_{};
    /// <summary>頂点バッファビュー (ID3D12GraphicsCommandList::IASetVertexBuffers 用)</summary>
    D3D12_VERTEX_BUFFER_VIEW vbView_{};

private:
    /// <summary>デバッグ用のメッシュ名</summary>
    std::string name_ = "UNKNOWN";

public:
    /// <summary>メッシュに名称を設定する（リソース名にも反映されることがある）.</summary>
    void SetName(const std::string& _name) {
        name_ = _name;
    }

    /// <summary>メッシュの名称を取得する.</summary>
    const std::string& GetName() const {
        return name_;
    }

    /// <summary>
    /// 頂点データのセット
    /// </summary>
    /// <param name="_data">データ</param>
    void SetVertexData(const std::vector<VertexDataType>& _data) {
        vertexes_ = _data;
    }

    /// <summary>
    /// インデックスデータのセット
    /// </summary>
    /// <param name="_data">データ</param>
    void SetIndexData(const std::vector<uint32_t>& _data) {
        indexes_ = _data;
    }

    /// <summary>
    /// 頂点バッファの取得
    /// </summary>
    /// <returns>バッファ</returns>
    DxResource& GetVertexBuffer() {
        return vertBuff_;
    }

    /// <summary>
    /// インデックスバッファの取得
    /// </summary>
    /// <returns>バッファ</returns>
    DxResource& GetIndexBuffer() {
        return indexBuff_;
    }

    /// <summary>
    /// 頂点キャパシティの取得
    /// </summary>
    /// <returns>キャパシティ</returns>
    int32_t GetVertexCapacity() const {
        return int32_t(vertexSize_) - int32_t(vertexes_.size());
    }

    /// <summary>
    /// インデックスキャパシティの取得
    /// </summary>
    /// <returns>キャパシティ</returns>
    int32_t GetIndexCapacity() const {
        return int32_t(indexSize_) - int32_t(indexes_.size());
    }

    /// <summary>
    /// 頂点数の取得
    /// </summary>
    /// <returns>頂点数</returns>
    uint32_t GetVertexSize() const {
        return vertexSize_;
    }

    /// <summary>
    /// インデックス数の取得
    /// </summary>
    /// <returns>インデックス数</returns>
    uint32_t GetIndexSize() const {
        return indexSize_;
    }

    /// <summary>
    /// 頂点数の設定
    /// </summary>
    /// <param name="_size">サイズ</param>
    void SetVertexSize(uint32_t _size) {
        vertexSize_ = _size;
        vertexes_.resize(vertexSize_);
    }

    /// <summary>
    /// インデックス数の設定
    /// </summary>
    /// <param name="_size">サイズ</param>
    void SetIndexSize(uint32_t _size) {
        indexSize_ = _size;
        indexes_.resize(indexSize_);
    }

    /// <summary>
    /// 頂点バッファビューの取得
    /// </summary>
    /// <returns>ビュー</returns>
    const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const {
        return vbView_;
    }

    /// <summary>
    /// インデックスバッファビューの取得
    /// </summary>
    /// <returns>ビュー</returns>
    const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() const {
        return ibView_;
    }

    /// <summary>
    /// 頂点バッファビューの取得
    /// </summary>
    /// <returns>ビュー</returns>
    const D3D12_VERTEX_BUFFER_VIEW& GetVBView() const {
        return vbView_;
    }

    /// <summary>
    /// インデックスバッファビューの取得
    /// </summary>
    /// <returns>ビュー</returns>
    const D3D12_INDEX_BUFFER_VIEW& GetIBView() const {
        return ibView_;
    }
};

using TextureMesh      = Mesh<TextureVertexData>;
using TextureColorMesh = Mesh<TextureColorVertexData>;
using PrimitiveMesh    = Mesh<PrimitiveVertexData>;

template <typename VertexDataType>
inline void Mesh<VertexDataType>::Initialize(UINT _vertexCapacity, UINT _indexCapacity) {

    this->vertexSize_ = _vertexCapacity;
    this->indexSize_  = _indexCapacity;

    if (this->vertexSize_ != 0) {
        vertexes_.resize(this->vertexSize_);

        UINT vertDataSize = sizeof(VertexDataType);

        vertBuff_.CreateBufferResource(Engine::GetInstance()->GetDxDevice()->device_, vertDataSize * this->vertexSize_);
        vbView_.BufferLocation = vertBuff_.GetResource()->GetGPUVirtualAddress();
        vbView_.SizeInBytes    = vertDataSize * this->vertexSize_;
        vbView_.StrideInBytes  = vertDataSize;
        vertBuff_.GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&vertData_));
    }

    if (this->indexSize_ != 0) {
        indexes_.resize(this->indexSize_);

        UINT indexDataSize = sizeof(uint32_t);
        indexBuff_.CreateBufferResource(Engine::GetInstance()->GetDxDevice()->device_, indexDataSize * this->indexSize_);
        ibView_.BufferLocation = indexBuff_.GetResource()->GetGPUVirtualAddress();
        ibView_.SizeInBytes    = indexDataSize * this->indexSize_;
        ibView_.Format         = DXGI_FORMAT_R32_UINT;
        indexBuff_.GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));
    }
}

template <typename VertexDataType>
inline void Mesh<VertexDataType>::Finalize() {
    vertBuff_.Finalize();
    indexBuff_.Finalize();
}

template <typename VertexDataType>
inline void Mesh<VertexDataType>::TransferData() {
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

template <typename VertexDataType>
inline void Mesh<VertexDataType>::copyVertexData(const VertexDataType* _data, uint32_t _size) {
    if (vertexSize_ < _size) {
        assert("vertexSize < _size");
        return;
    }

    if (_size > vertexes_.size()) {
        vertexes_.resize(_size);
    }
    memcpy(vertexes_.data(), _data, sizeof(VertexDataType) * _size);
}

template <typename VertexDataType>
inline void Mesh<VertexDataType>::copyIndexData(const uint32_t* _data, uint32_t _size) {
    if (indexSize_ < _size) {
        assert("indexCapacity_ < _size");
    }
    if (_size > indexes_.size()) {
        indexes_.resize(_size);
    }
    memcpy(indexes_.data(), _data, sizeof(uint32_t) * _size);
}

} // namespace OriGine

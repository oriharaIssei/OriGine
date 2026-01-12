#pragma once

/// stl & api
#include <cstdint>
#include <d3d12.h>
#include <memory>

/// engine
// directX12
#include "ConceptHasBuffer.h"
#include "directX12/DxResource.h"

/// util
#include <logger/Logger.h>

namespace OriGine {

/// <summary>
/// GPU上の定数バッファ (ConstantBuffer) と、それに対応する CPU 上のデータを一括管理するクラス.
/// テンプレート引数 T は、内部に 'ConstantBuffer' という名前の構造体を持つことを想定している.
/// </summary>
/// <typeparam name="constBuff">定数バッファのデータ構造を定義したクラス/構造体</typeparam>
template <HasInConstantBuffer constBuff>
class IConstantBuffer {
public:
    /// <summary>
    /// コンストラクタ. 内部データの初期化引数を渡すことができる.
    /// </summary>
    template <typename... Args>
    IConstantBuffer(Args... args) {
        openData_ = constBuff(args...);
    }

    IConstantBuffer() {}
    ~IConstantBuffer() {}

    /// <summary>
    /// DirectX 12 側のバッファリソースを作成し、CPU空間へのマッピングを開始する.
    /// </summary>
    /// <param name="device">D3D12デバイス</param>
    void CreateBuffer(Microsoft::WRL::ComPtr<ID3D12Device> device);

    /// <summary>
    /// バッファリソースの解放とマッピングの解除を行う.
    /// </summary>
    inline void Finalize();

    /// <summary>
    /// CPU 上のデータ (openData_) を GPU 用の定数バッファに転送（コピー）する.
    /// </summary>
    void ConvertToBuffer() const;

    /// <summary>
    /// グラフィックスパイプラインのルートパラメータに対して、この定数バッファを紐付ける.
    /// </summary>
    /// <param name="cmdList">コマンドリスト</param>
    /// <param name="rootParameterNum">バインド先のルートパラメータ番号</param>
    void SetForRootParameter(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList, uint32_t rootParameterNum) const;

    /// <summary>
    /// コンピュートパイプラインのルートパラメータに対して、この定数バッファを紐付ける.
    /// </summary>
    /// <param name="cmdList">コマンドリスト</param>
    /// <param name="rootParameterNum">バインド先のルートパラメータ番号</param>
    void SetForComputeRootParameter(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList, uint32_t rootParameterNum) const;

public:
    /// <summary>CPU側で操作可能な生データへのアクセス</summary>
    constBuff openData_;

    /// <summary>メンバアクセス演算子のオーバーロード</summary>
    constBuff* operator->() { return &openData_; }
    /// <summary>メンバアクセス演算子のオーバーロード (const)</summary>
    const constBuff* operator->() const { return &openData_; }
    /// <summary>ポインタ型への暗黙キャスト</summary>
    operator const constBuff*() const { return &openData_; }

protected:
    /// <summary>GPUバッファに対応するマッピングされたアドレス</summary>
    typename constBuff::ConstantBuffer* mappingData_ = nullptr;
    /// <summary>D3D12リソース管理オブジェクト</summary>
    DxResource buff_;

public:
    /// <summary>DxResource オブジェクトへの参照を取得する.</summary>
    const DxResource& GetResource() const { return buff_; }
};

template <HasInConstantBuffer constBuff>
inline void IConstantBuffer<constBuff>::CreateBuffer(Microsoft::WRL::ComPtr<ID3D12Device> device) {
    // リソースがすでに存在する場合は何もしない
    if (buff_.GetResourceRef().Get()) {
        LOG_WARN("IConstantBuffer: Buffer resource already exists. Skipping creation.");
        return;
    }

    // バッファリソースの作成
    buff_.CreateBufferResource(device, sizeof(constBuff::ConstantBuffer));
    buff_.GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&mappingData_));

    // リソースタイプの設定
    buff_.SetType(DxResourceType::Buffer_Constant);
}

template <HasInConstantBuffer constBuff>
inline void IConstantBuffer<constBuff>::Finalize() {
    // buffer が有効な場合は終了処理を行う
    if (buff_.IsValid()) {
        buff_.Finalize();
    }
}
template <HasInConstantBuffer constBuff>
inline void IConstantBuffer<constBuff>::ConvertToBuffer() const {
    *mappingData_ = openData_;
}

template <HasInConstantBuffer constBuff>
inline void IConstantBuffer<constBuff>::SetForRootParameter(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList, uint32_t rootParameterNum) const {
    cmdList->SetGraphicsRootConstantBufferView(rootParameterNum, buff_.GetResource()->GetGPUVirtualAddress());
}

template <HasInConstantBuffer constBuff>
inline void IConstantBuffer<constBuff>::SetForComputeRootParameter(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList, uint32_t rootParameterNum) const {
    cmdList->SetComputeRootConstantBufferView(rootParameterNum, buff_.GetResource()->GetGPUVirtualAddress());
}

} // namespace OriGine

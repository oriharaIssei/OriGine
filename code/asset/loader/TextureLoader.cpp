#include "TextureLoader.h"

/// directXTex
#include <DirectXTex/d3dx12.h>

/// stl
#include <cassert>

/// engine
#include "Engine.h"
// directX12
#include "directX12/DxDescriptor.h"
#include "directX12/DxDevice.h"
#include "directX12/DxFence.h"

/// util
#include "util/StringUtil.h"

using namespace OriGine;

/// <summary>
/// CPU側に読み込み済みの画像をGPUテクスチャへ転送し、SRVまで作成する。
/// WIC/DDSどちらのローダーからも呼ばれる共通処理
/// </summary>
/// <param name="_dxCommand">転送に使うコマンドリスト（この関数内で実行・リセットされる）</param>
/// <param name="_image">転送元の画像データ（ミップマップ生成済み）</param>
/// <param name="_outAsset">生成したリソース・メタデータ・SRVの格納先</param>
void TextureUploadHelper::UploadToGpu(
    DxCommand* _dxCommand,
    const DirectX::ScratchImage& _image,
    TextureAsset& _outAsset) {
    auto dxDevice = Engine::GetInstance()->GetDxDevice();

    // Resource 作成
    _outAsset.resource.CreateTextureResource(
        dxDevice->device_,
        _image.GetMetadata());

    std::vector<D3D12_SUBRESOURCE_DATA> subResources;
    DirectX::PrepareUpload(
        dxDevice->device_.Get(),
        _image.GetImages(),
        _image.GetImageCount(),
        _image.GetMetadata(),
        subResources);

    uint64_t uploadSize = GetRequiredIntermediateSize(
        _outAsset.resource.GetResource().Get(),
        0,
        UINT(subResources.size()));

    DxResource uploadBuffer;
    uploadBuffer.CreateBufferResource(dxDevice->device_, uploadSize);

    UpdateSubresources(
        _dxCommand->GetCommandList().Get(),
        _outAsset.resource.GetResource().Get(),
        uploadBuffer.GetResource().Get(),
        0,
        0,
        UINT(subResources.size()),
        subResources.data());

    // Barrier & Execute
    // UpdateSubresources 直後のリソースは CPU→GPU コピーの書き込み先(COPY_DEST)状態のままなので、
    // シェーダから読み取れる状態(GENERIC_READ = テクスチャ用の読み取り専用汎用状態)へ遷移させる。
    // これを行わないままSRVを介して読み込むとドライバのバリデーションレイヤーでエラーになる。
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource   = _outAsset.resource.GetResource().Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_GENERIC_READ;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    _dxCommand->GetResourceStateTracker()
        ->DirectBarrier(_dxCommand->GetCommandList(),
            _outAsset.resource.GetResource().Get(),
            barrier);

    _dxCommand->Close();
    _dxCommand->ExecuteCommand();

    // アップロード用コマンドをこの場で即座に実行し、フェンスで GPU の完了を待つ（同期的アップロード）。
    // uploadBuffer はこの関数を抜けるとスコープアウトして解放されるため、
    // GPU 側のコピーが完了する前に解放してしまわないよう、ここで確実に待機する必要がある。
    DxFence* fence = Engine::GetInstance()->GetDxFence();
    UINT64 val     = fence->Signal(_dxCommand->GetCommandQueue());
    fence->WaitForFence(val);

    _dxCommand->CommandReset();

    _outAsset.metaData = _image.GetMetadata();

    // SRV の作成
    /// metadataを基に SRV の設定
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format                  = _outAsset.metaData.format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    if (_outAsset.metaData.IsCubemap()) {
        // キューブマップは6面をまとめて1つの TEXTURECUBE ビューとして扱うため、
        // 2Dテクスチャとはビューの種類が異なる。MipLevels に UINT_MAX を指定することで
        // 実際に確保されているミップレベル数まで自動的に対象にする
        srvDesc.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.TextureCube.MostDetailedMip     = 0;
        srvDesc.TextureCube.MipLevels           = UINT_MAX;
        srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
    } else {
        // 通常の2Dテクスチャの場合は、実際に生成されたミップレベル数を明示的に指定する
        srvDesc.ViewDimension       = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = UINT(_outAsset.metaData.mipLevels);
    }
    SRVEntry srvEntry(&_outAsset.resource, srvDesc);
    _outAsset.srv = Engine::GetInstance()->GetSrvHeap()->CreateDescriptor(&srvEntry);
}

/// <summary>
/// 初期化処理。テクスチャ転送専用のコマンドリストを用意する
/// （描画用のコマンドリストとは独立させ、読み込み時に即時実行できるようにするため）
/// </summary>
void OriGine::TextureWicLoader::Initialize() {
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");
}

/// <summary>
/// 終了処理。転送用コマンドリストを解放する
/// </summary>
void OriGine::TextureWicLoader::Finalize() {
    dxCommand_->Finalize();
    dxCommand_.reset();
}

/// <summary>
/// WICが扱える画像形式（png/jpg等）を読み込み、ミップマップを生成してGPUへ転送する
/// </summary>
/// <param name="_assetPath">読み込むファイルのパス</param>
/// <returns>リソース・メタデータ・SRVを保持したTextureAsset</returns>
TextureAsset TextureWicLoader::LoadAsset(const std::string& _assetPath) {
    TextureAsset asset{};

    DirectX::ScratchImage _image{};
    DirectX::ScratchImage mipImages{};

    std::wstring pathW = ConvertString(_assetPath);

    HRESULT hr = DirectX::LoadFromWICFile(
        pathW.c_str(),
        DirectX::WIC_FLAGS_FORCE_SRGB | DirectX::WIC_FLAGS_DEFAULT_SRGB,
        &asset.metaData,
        _image);
    assert(SUCCEEDED(hr));

    // WIC（png/jpg等の一般的な画像形式）はミップマップ情報を持たないため、ロード時に自前で生成する。
    // 1x1 の画像はこれ以上縮小できないため対象外とする
    if (_image.GetMetadata().width > 1 && _image.GetMetadata().height > 1) {
        hr = DirectX::GenerateMipMaps(
            _image.GetImages(),
            _image.GetImageCount(),
            _image.GetMetadata(),
            DirectX::TEX_FILTER_SRGB,
            0,
            mipImages);
        assert(SUCCEEDED(hr));
    } else {
        mipImages = std::move(_image);
    }

    asset.metaData = mipImages.GetMetadata();

    TextureUploadHelper::UploadToGpu(dxCommand_.get(), mipImages, asset);
    return asset;
}

/// <summary>
/// 初期化処理。テクスチャ転送専用のコマンドリストを用意する
/// </summary>
void OriGine::TextureDdsLoader::Initialize() {
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");
}

/// <summary>
/// 終了処理。転送用コマンドリストを解放する
/// </summary>
void OriGine::TextureDdsLoader::Finalize() {
    dxCommand_->Finalize();
    dxCommand_.reset();
}

/// <summary>
/// クック済みのDDSファイルを読み込みGPUへ転送する。
/// DDSはビルド時にミップマップ生成と圧縮を済ませてあるため、
/// WIC版と違い実行時のミップ生成（GenerateMipMaps）が不要で、その分読み込みが速い
/// </summary>
/// <param name="_assetPath">読み込むDDSファイルのパス</param>
/// <returns>リソース・メタデータ・SRVを保持したTextureAsset</returns>
TextureAsset TextureDdsLoader::LoadAsset(const std::string& _assetPath) {
    TextureAsset asset{};

    DirectX::ScratchImage _image{};
    std::wstring pathW = ConvertString(_assetPath);

    HRESULT hr = DirectX::LoadFromDDSFile(
        pathW.c_str(),
        DirectX::DDS_FLAGS_NONE,
        &asset.metaData,
        _image);
    assert(SUCCEEDED(hr));

    // DDS は mipmap 済み前提
    TextureUploadHelper::UploadToGpu(dxCommand_.get(), _image, asset);
    return asset;
}

#pragma once

/// stl
// memory
#include <memory>
// container
#include <queue>
#include <unordered_map>
// basic
#include <stdint.h>
#include <string>

/// microsoft
#include <wrl.h>

/// engine
// dx12Object
#include "directX12/DxCommand.h"
#include "directX12/DxDescriptor.h"
#include "directX12/DxFence.h"
#include "directX12/DxResource.h"
#include "directX12/PipelineStateObj.h"

namespace OriGine {
/// <summary>
/// 個別のテクスチャリソースとその情報を保持する構造体.
/// </summary>
struct Texture {
    /// <summary>
    /// テクスチャの初期化を行う. ファイルから読み込み、GPUリソースを作成する.
    /// </summary>
    /// <param name="_filePath">読み込み対象のファイルパス</param>
    void Initialize(const std::string& _filePath);

    /// <summary>
    /// テクスチャリソースの解放を行う.
    /// </summary>
    void Finalize();

    /// <summary>ファイルパス</summary>
    std::string path;
    /// <summary>テクスチャのメタデータ（幅、高さ、形式等）</summary>
    DirectX::TexMetadata metaData;
    /// <summary>DirectX12 リソースオブジェクト</summary>
    DxResource resource;
    /// <summary>SRV（シェーダーリソースビュー）記述子</summary>
    DxSrvDescriptor srv;

private:
    /// <summary>
    /// 画像ファイルをディスクから読み込む.
    /// </summary>
    /// <param name="_filePath">ファイルパス</param>
    /// <returns>読み込まれた画像イメージ</returns>
    DirectX::ScratchImage Load(const std::string& _filePath);

    /// <summary>
    /// 画像データを GPU のアップロードバッファへ転送し、最終的なリソースへコピーする準備をする.
    /// </summary>
    /// <param name="_mipImg">画像イメージ（ミップマップ含む）</param>
    /// <param name="_resource">転送先のリソース</param>
    void UploadTextureData(DirectX::ScratchImage& _mipImg, Microsoft::WRL::ComPtr<ID3D12Resource> _resource);

    /// <summary>
    /// テクスチャアップロード用のコマンドリストを実行し、完了を待機する.
    /// </summary>
    /// <param name="_resource">リソース</param>
    void ExecuteCommand(Microsoft::WRL::ComPtr<ID3D12Resource> _resource);
};

/// <summary>
/// テクスチャのロード・アンロードおよびリソース管理を司るシングルトンライクな静的クラス.
/// </summary>
class TextureManager {
    friend struct Texture;

public:
    TextureManager();
    ~TextureManager();

    /// <summary>
    /// テクスチャ管理システムの初期化を行う.
    /// </summary>
    static void Initialize();

    /// <summary>
    /// テクスチャ管理システムの終了処理を行う. 全テクスチャを解放する.
    /// </summary>
    static void Finalize();

    /// <summary>
    /// テクスチャをロードし、管理用のハンドル（インデックス）を返す.
    /// 同じファイル名が既にロードされている場合は、そのハンドルを再利用する.
    /// </summary>
    /// <param name="_filePath">ロードするファイルパス</param>
    /// <param name="_callBack">ロード完了時に呼び出されるコールバック関数（任意）</param>
    /// <returns>テクスチャハンドル. ロード失敗時はダミーテクスチャのハンドルを返すことがある.</returns>
    static uint32_t LoadTexture(const std::string& _filePath, std::function<void(uint32_t)> _callBack = nullptr);

    /// <summary>
    /// 指定されたハンドルに対応するテクスチャをアンロードする.
    /// </summary>
    /// <param name="_id">テクスチャハンドル</param>
    static void UnloadTexture(uint32_t _id);

public:
    /// <summary>管理可能な最大テクスチャ数</summary>
    static const uint32_t maxTextureSize_ = 192;

    /// <summary>
    /// テクスチャロードタスク情報を保持する構造体.
    /// </summary>
    struct LoadTask {
        /// <summary>対象ファイルパス</summary>
        std::string filePath;
        /// <summary>割り当てられたインデックス</summary>
        uint32_t textureIndex = 0;
        /// <summary>テクスチャオブジェクトの共有ポインタ</summary>
        std::shared_ptr<Texture> texture = nullptr;

        /// <summary>完了報知用コールバック</summary>
        std::function<void(uint32_t)> callBack;

        /// <summary>
        /// タスクの更新・実行処理.
        /// </summary>
        void Update();
    };

private:
    /// <summary>ロード済みテクスチャの配列</summary>
    static std::array<std::shared_ptr<Texture>, maxTextureSize_> textures_;
    /// <summary>ファイルパスからハンドルへの逆引きマップ</summary>
    static std::unordered_map<std::string, uint32_t> textureFileNameToIndexMap_;
    /// <summary>ダミー（1x1白色）テクスチャのハンドル</summary>
    static uint32_t dummyTextureIndex_;

    /// <summary>アップロード用コマンドオブジェクト</summary>
    static std::unique_ptr<DxCommand> dxCommand_;

public:
    /// <summary>
    /// 指定されたハンドルのテクスチャに対応する GPU 記述子ハンドルを取得する.
    /// </summary>
    /// <param name="_handleId">テクスチャハンドル</param>
    /// <returns>GPU 記述子ハンドル</returns>
    static D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorGpuHandle(uint32_t _handleId);

    /// <summary>
    /// 指定されたハンドルのテクスチャのメタデータを取得する.
    /// </summary>
    /// <param name="_handleId">テクスチャハンドル</param>
    /// <returns>テクスチャメタデータ</returns>
    static const DirectX::TexMetadata& GetTexMetadata(uint32_t _handleId) { return textures_[_handleId]->metaData; }
};

// engine/resource/ は含まれない
const static std::string kDefaultWhiteTextureLocalPath = "/Texture/white1x1.png";

} // namespace OriGine

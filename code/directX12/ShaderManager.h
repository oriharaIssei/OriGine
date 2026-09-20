#pragma once

/// Microsoft
#include <wrl.h>
/// stl
// memory
#include <memory>

// container
#include <array>
#include <unordered_map>
#include <vector>
// string
#include <string>
// exception
#include <dxcapi.h>
#include <stdexcept>
#include <stdint.h>

/// engine
#define RESOURCE_DIRECTORY
#include "EngineInclude.h"
// asset
#include "asset/manager/AssetManager.h"
#include "asset/ShaderAsset.h"
// dx12object
#include "directX12/BlendMode.h"
#include "directX12/PipelineStateObj.h"

/// DLL境界
#include "OriGineApi.h"

namespace OriGine {

const std::string kShaderDirectory = "engine/resource/Shader";

class ShaderManager;
/// <summary>
/// パイプラインステートオブジェクト (PSO) を生成するためのパラメータを保持・構築するクラス.
/// 各種描画設定（デプス、カリング、ルートシグネチャ要素）をメソッドチェーンや逐次追加形式で設定できる.
/// </summary>
class ShaderInformation {
    friend class ShaderManager;

private:
    /// <summary>深度ステンシル設定（デフォルトは有効・比較条件 LessEqual）</summary>
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc_{
        .DepthEnable    = true,
        .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL,
        .DepthFunc      = D3D12_COMPARISON_FUNC_LESS_EQUAL};

    /// <summary>静的サンプラー設定のリスト</summary>
    std::vector<D3D12_STATIC_SAMPLER_DESC> samplerDescs_;
    /// <summary>ルートパラメータ（定数・記述子テーブル）のリスト</summary>
    std::vector<D3D12_ROOT_PARAMETER> rootParameters_;
    /// <summary>記述子テーブル内の範囲（ディスクリプタレンジ）の管理</summary>
    std::vector<std::unique_ptr<D3D12_DESCRIPTOR_RANGE[]>> descriptorRanges_;
    /// <summary>頂点レイアウト設定のリスト</summary>
    std::vector<D3D12_INPUT_ELEMENT_DESC> elementDescs_;

    /// <summary>ラスタライザ設定</summary>
    D3D12_RASTERIZER_DESC rasterizerDesc{
        .FillMode = D3D12_FILL_MODE_SOLID,
        .CullMode = D3D12_CULL_MODE_BACK};

public:
    /// <summary>プリミティブトポロジーの種類（デフォルトは三角形）</summary>
    D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    /// <summary>ルートシグネチャのフラグ設定</summary>
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlag = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    /// <summary>使用する頂点シェーダーのキー名（ShaderManager に登録済みを想定）</summary>
    std::string vsKey = "";
    /// <summary>使用するピクセルシェーダーのキー名</summary>
    std::string psKey = "";
    /// <summary>使用するコンピュートシェーダーのキー名</summary>
    std::string csKey = "";
    /// <summary>使用するドメインシェーダーのキー名</summary>
    std::string dsKey = "";
    /// <summary>使用するハルシェーダーのキー名</summary>
    std::string hsKey = "";
    /// <summary>使用するジオメトリシェーダーのキー名</summary>
    std::string gsKey = "";

    /// <summary>ブレンドモード設定</summary>
    BlendMode blendMode_ = BlendMode::Alpha;

    /// <summary>
    /// 静的サンプラー設定を追加する.
    /// </summary>
    /// <param name="_samplerDesc">サンプラー設定</param>
    /// <returns>追加されたインデックス</returns>
    size_t pushBackSamplerDesc(const D3D12_STATIC_SAMPLER_DESC& _samplerDesc) {
        samplerDescs_.emplace_back(_samplerDesc);
        return samplerDescs_.size() - 1;
    }

    /// <summary>
    /// ルートパラメータを追加する.
    /// </summary>
    /// <param name="_parameter">パラメータ設定（定数や記述子テーブル）</param>
    /// <returns>追加されたインデックス</returns>
    size_t pushBackRootParameter(const D3D12_ROOT_PARAMETER& _parameter) {
        rootParameters_.push_back(_parameter);
        return rootParameters_.size() - 1;
    }

    /// <summary>
    /// 頂点レイアウト設定を追加する.
    /// </summary>
    /// <param name="_elementDesc">レイアウト要素</param>
    /// <returns>追加されたインデックス</returns>
    size_t pushBackInputElementDesc(const D3D12_INPUT_ELEMENT_DESC& _elementDesc) {
        elementDescs_.push_back(_elementDesc);
        return elementDescs_.size() - 1;
    }

    /// <summary>
    /// 既存のルートパラメータ（記述子テーブル）に対してディスクリプタレンジを設定する.
    /// パラメータ追加後に呼び出す必要がある.
    /// </summary>
    /// <param name="_range">レンジ配列の先頭ポインタ</param>
    /// <param name="_numRanges">レンジ数</param>
    /// <param name="_rootParameterIndex">対象とするパラメータのインデックス</param>
    void SetDescriptorRange2Parameter(const D3D12_DESCRIPTOR_RANGE* _range, size_t _numRanges, size_t _rootParameterIndex) {
        // 動的に確保して管理する
        auto ranges = std::make_unique<D3D12_DESCRIPTOR_RANGE[]>(_numRanges);
        std::copy(_range, _range + _numRanges, ranges.get());

        descriptorRanges_.push_back(std::move(ranges));
        rootParameters_[_rootParameterIndex].DescriptorTable.pDescriptorRanges   = descriptorRanges_.back().get();
        rootParameters_[_rootParameterIndex].DescriptorTable.NumDescriptorRanges = static_cast<UINT>(_numRanges);
    }

    /// <summary>
    /// カリングモードを変更する.
    /// </summary>
    /// <param name="_cullMode">D3D12_CULL_MODE</param>
    void changeCullMode(D3D12_CULL_MODE _cullMode) {
        rasterizerDesc.CullMode = _cullMode;
    }

    /// <summary>
    /// ラスタライザのフィルモードを変更する.
    /// </summary>
    /// <param name="_fillMode">D3D12_FILL_MODE (SOLID, WIREFRAME)</param>
    void changeFillMode(D3D12_FILL_MODE _fillMode) {
        rasterizerDesc.FillMode = _fillMode;
    }

    /// <summary>
    /// 深度ステンシル設定を直接編集するための参照を取得する（設定の自由度優先）.
    /// </summary>
    /// <returns>設定構造体への参照</returns>
    D3D12_DEPTH_STENCIL_DESC& CustomDepthStencilDesc() { return depthStencilDesc_; }

    /// <summary>
    /// 深度ステンシル設定を一括で上書きする.
    /// </summary>
    /// <param name="_desc">設定構造体</param>
    void SetDepthStencilDesc(const D3D12_DEPTH_STENCIL_DESC& _desc) { depthStencilDesc_ = _desc; }

    /// <summary>
    /// プリミティブトポロジーの種類を設定する.
    /// </summary>
    /// <param name="_topology">D3D12_PRIMITIVE_TOPOLOGY_TYPE</param>
    void SetTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE _topology) { topologyType = _topology; }
};
using ShaderInfo = ShaderInformation;

/// <summary>
/// パイプラインステートオブジェクト (PSO) の一元管理を行うシングルトンクラス.
///
/// シェーダーバイナリそのものの読み込み・コンパイル・キャッシュは
/// ShaderAssetManager が担当する. このクラスは
///   - 「短いキー名 ("Object3dTexture.VS" 等) → シェーダーアセット」の対応付け
///   - ShaderInformation から生成した PSO のキャッシュ
/// を受け持つ.
/// </summary>
class ORIGINE_API ShaderManager {
public:
    /// <summary>
    /// シングルトンインスタンスを取得する.
    /// .cpp に出しているのは DxDebug と同じ理由(docs/plans/phase-04.md 4D 3):
    /// inlineのままだとDLLとEXEの両方に別々の実体(関数ローカルstatic)が生まれる。
    /// </summary>
    static ShaderManager* GetInstance();

public:
    /// <summary>
    /// マネージャの初期化を行う.
    /// シェーダーのコンパイラ本体は ShaderAssetManager 側が保持するため、
    /// ここで行うのはキャッシュのクリアのみ.
    /// </summary>
    void Initialize();

    /// <summary>
    /// PSO を解放し、参照していたシェーダーアセットを手放す.
    /// AssetSystem::Finalize より先に呼ぶこと.
    /// </summary>
    void Finalize();

    /// <summary>
    /// 指定された ShaderInformation に基づいて PipelineStateObj (PSO) を生成し、登録する.
    /// 既に同じキーで登録されている場合は、生成を行わず既存のポインタを返す.
    /// </summary>
    /// <param name="_key">PSO を識別する一意のキー</param>
    /// <param name="_shaderInfo">生成パラメータ</param>
    /// <param name="_device">使用するD3D12デバイス</param>
    /// <returns>生成（または取得）された PSO のポインタ</returns>
    PipelineStateObj* CreatePso(const std::string& _key, const ShaderInformation& _shaderInfo, Microsoft::WRL::ComPtr<ID3D12Device> _device);

    /// <summary>
    /// 指定されたシェーダーファイルを読み込み、コンパイル結果を ShaderAssetManager 経由でキャッシュする.
    /// 同じファイル・同じプロファイルの組み合わせは再コンパイルされない.
    /// </summary>
    /// <param name="_fileName">シェーダーファイル名（拡張子 .hlsl は付けない。これがそのままキーになる）</param>
    /// <param name="_directory">ファイルが存在するディレクトリのパス</param>
    /// <param name="_profile">コンパイルプロファイル（vs_6_0, ps_6_0 など）</param>
    /// <returns>成功した場合は true</returns>
    bool LoadShader(const std::string& _fileName, const std::string& _directory = kShaderDirectory, const wchar_t* _profile = L"vs_6_0");

private:
    ShaderManager() = default;
    ~ShaderManager() {};
    ShaderManager(const ShaderManager&)                  = delete;
    const ShaderManager& operator=(const ShaderManager&) = delete;

    /// <summary>
    /// ShaderAssetManager を取得する.
    /// </summary>
    /// <returns>未登録の場合は nullptr</returns>
    static AssetManager<ShaderAsset>* GetShaderAssetManager();

private:
    /// <summary>短いキー名（"Object3dTexture.VS" 等）とシェーダーアセットインデックスのマップ</summary>
    std::unordered_map<std::string, size_t> shaderKeyToAssetIndex_;
    /// <summary>キー名と生成済み PSO のマップ</summary>
    std::unordered_map<std::string, std::unique_ptr<PipelineStateObj>> psoMap_;

public:
    /// <summary>
    /// 指定されたシェーダーが既に読み込み済みかを確認する.
    /// </summary>
    bool IsRegisteredShaderBlob(const std::string& _key) const {
        return shaderKeyToAssetIndex_.find(_key) != shaderKeyToAssetIndex_.end();
    }

    /// <summary>
    /// 指定されたキーの PSO が登録済みかを確認する.
    /// </summary>
    bool IsRegisteredPipelineStateObj(const std::string& _key) const {
        return psoMap_.find(_key) != psoMap_.end();
    }

    /// <summary>
    /// 登録済みの PSO を取得する.
    /// </summary>
    /// <param name="_key">識別キー</param>
    /// <returns>PSO ポインタ. 存在しない場合は nullptr.</returns>
    PipelineStateObj* GetPipelineStateObj(const std::string& _key);

    /// <summary>
    /// 読み込み済みのシェーダーバイナリを取得する.
    /// </summary>
    /// <param name="_key">識別キー（LoadShader に渡したファイル名）</param>
    /// <returns>ブロブポインタ. 存在しない場合は nullptr.</returns>
    IDxcBlob* GetShaderBlob(const std::string& _key) const;
};

} // namespace OriGine

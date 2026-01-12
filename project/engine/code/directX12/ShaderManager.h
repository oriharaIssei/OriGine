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
// dx12object
#include "directX12/BlendMode.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderCompiler.h"

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
    /// <param name="samplerDesc">サンプラー設定</param>
    /// <returns>追加されたインデックス</returns>
    size_t pushBackSamplerDesc(const D3D12_STATIC_SAMPLER_DESC& samplerDesc) {
        samplerDescs_.emplace_back(samplerDesc);
        return samplerDescs_.size() - 1;
    }

    /// <summary>
    /// ルートパラメータを追加する.
    /// </summary>
    /// <param name="parameter">パラメータ設定（定数や記述子テーブル）</param>
    /// <returns>追加されたインデックス</returns>
    size_t pushBackRootParameter(const D3D12_ROOT_PARAMETER& parameter) {
        rootParameters_.push_back(parameter);
        return rootParameters_.size() - 1;
    }

    /// <summary>
    /// 頂点レイアウト設定を追加する.
    /// </summary>
    /// <param name="elementDesc">レイアウト要素</param>
    /// <returns>追加されたインデックス</returns>
    size_t pushBackInputElementDesc(const D3D12_INPUT_ELEMENT_DESC& elementDesc) {
        elementDescs_.push_back(elementDesc);
        return elementDescs_.size() - 1;
    }

    /// <summary>
    /// 既存のルートパラメータ（記述子テーブル）に対してディスクリプタレンジを設定する.
    /// パラメータ追加後に呼び出す必要がある.
    /// </summary>
    /// <param name="range">レンジ配列の先頭ポインタ</param>
    /// <param name="numRanges">レンジ数</param>
    /// <param name="rootParameterIndex">対象とするパラメータのインデックス</param>
    void SetDescriptorRange2Parameter(const D3D12_DESCRIPTOR_RANGE* range, size_t numRanges, size_t rootParameterIndex) {
        // 動的に確保して管理する
        auto ranges = std::make_unique<D3D12_DESCRIPTOR_RANGE[]>(numRanges);
        std::copy(range, range + numRanges, ranges.get());

        descriptorRanges_.push_back(std::move(ranges));
        rootParameters_[rootParameterIndex].DescriptorTable.pDescriptorRanges   = descriptorRanges_.back().get();
        rootParameters_[rootParameterIndex].DescriptorTable.NumDescriptorRanges = static_cast<UINT>(numRanges);
    }

    /// <summary>
    /// カリングモードを変更する.
    /// </summary>
    /// <param name="cullMode">D3D12_CULL_MODE</param>
    void changeCullMode(D3D12_CULL_MODE cullMode) {
        rasterizerDesc.CullMode = cullMode;
    }

    /// <summary>
    /// ラスタライザのフィルモードを変更する.
    /// </summary>
    /// <param name="fillMode">D3D12_FILL_MODE (SOLID, WIREFRAME)</param>
    void changeFillMode(D3D12_FILL_MODE fillMode) {
        rasterizerDesc.FillMode = fillMode;
    }

    /// <summary>
    /// 深度ステンシル設定を直接編集するための参照を取得する（設定の自由度優先）.
    /// </summary>
    /// <returns>設定構造体への参照</returns>
    D3D12_DEPTH_STENCIL_DESC& CustomDepthStencilDesc() { return depthStencilDesc_; }

    /// <summary>
    /// 深度ステンシル設定を一括で上書きする.
    /// </summary>
    /// <param name="desc">設定構造体</param>
    void SetDepthStencilDesc(const D3D12_DEPTH_STENCIL_DESC& desc) { depthStencilDesc_ = desc; }

    /// <summary>
    /// プリミティブトポロジーの種類を設定する.
    /// </summary>
    /// <param name="topology">D3D12_PRIMITIVE_TOPOLOGY_TYPE</param>
    void SetTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology) { topologyType = topology; }
};
using ShaderInfo = ShaderInformation;

/// <summary>
/// シェーダーのコンパイル結果と、それに基づく PSO の一元管理を行うシングルトンクラス.
/// </summary>
class ShaderManager {
public:
    /// <summary>
    /// シングルトンインスタンスを取得する.
    /// </summary>
    static ShaderManager* GetInstance() {
        static ShaderManager instance;
        return &instance;
    }

public:
    /// <summary>
    /// マネージャの初期化を行う. シェーダーコンパイラの生成などを含む.
    /// </summary>
    void Initialize();

    /// <summary>
    /// 全てのキャッシュ（シェーダーブロブ、PSO）を解放する.
    /// </summary>
    void Finalize();

    /// <summary>
    /// 指定された ShaderInformation に基づいて PipelineStateObj (PSO) を生成し、登録する.
    /// 既に同じキーで登録されている場合は、生成を行わず既存のポインタを返す.
    /// </summary>
    /// <param name="key">PSO を識別する一意のキー</param>
    /// <param name="shaderInfo">生成パラメータ</param>
    /// <param name="device">使用するD3D12デバイス</param>
    /// <returns>生成（または取得）された PSO のポインタ</returns>
    PipelineStateObj* CreatePso(const std::string& key, const ShaderInformation& shaderInfo, Microsoft::WRL::ComPtr<ID3D12Device> device);

    /// <summary>
    /// 指定されたシェーダーファイルを読み込み、コンパイルしてメモリにキャッシュする.
    /// </summary>
    /// <param name="fileName">シェーダーファイル名</param>
    /// <param name="directory">ファイルが存在するディレクトリのパス</param>
    /// <param name="profile">コンパイルプロファイル（vs_6_0, ps_6_0 など）</param>
    /// <returns>成功した場合は true</returns>
    bool LoadShader(const std::string& fileName, const std::string& directory = kShaderDirectory, const wchar_t* profile = L"vs_6_0");

private:
    ShaderManager() = default;
    ~ShaderManager() {};
    ShaderManager(const ShaderManager&)                  = delete;
    const ShaderManager& operator=(const ShaderManager&) = delete;

private:
    /// <summary>ファイル名（またはキー）とコンパイル済みシェーダーブロブのマップ</summary>
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<IDxcBlob>> shaderBlobMap_;
    /// <summary>コンパイル処理を行うヘルパークラス</summary>
    std::unique_ptr<ShaderCompiler> shaderCompiler_;
    /// <summary>キー名と生成済み PSO のマップ</summary>
    std::unordered_map<std::string, std::unique_ptr<PipelineStateObj>> psoMap_;

public:
    /// <summary>
    /// シェーダーコンパイラへのポインタを取得する.
    /// </summary>
    ShaderCompiler* GetShaderCompiler() const { return shaderCompiler_.get(); }

    /// <summary>
    /// コンパイル済みのシェーダーブロブを外部から登録する.
    /// </summary>
    /// <param name="fileName">登録名（キー）</param>
    /// <param name="shaderBlob">コンパイル済みデータ</param>
    /// <returns>新規登録された場合は true</returns>
    bool RegisterShaderBlob(const std::string& fileName, Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob);

    /// <summary>
    /// 指定されたシェーダーが既にコンパイル・登録されているかを確認する.
    /// </summary>
    bool IsRegisteredShaderBlob(const std::string& fileName) const;

    /// <summary>
    /// シェーダーブロブを強制的に上書き、または新規登録する.
    /// </summary>
    void ForciblyRegisterShaderBlob(const std::string& fileName, Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob) {
        shaderBlobMap_[fileName] = std::move(shaderBlob);
    };

    /// <summary>
    /// 指定されたキーの PSO が登録済みかを確認する.
    /// </summary>
    bool IsRegisteredPipelineStateObj(const std::string& key) const {
        return psoMap_.find(key) != psoMap_.end();
    }

    /// <summary>
    /// 登録済みの PSO を取得する.
    /// </summary>
    /// <param name="key">識別キー</param>
    /// <returns>PSO ポインタ. 存在しない場合は nullptr.</returns>
    PipelineStateObj* GetPipelineStateObj(const std::string& key);

    /// <summary>
    /// 登録済みのシェーダーブロブを取得する.
    /// </summary>
    /// <param name="key">識別キー（ファイル名など）</param>
    /// <returns>ブロブデータポインタ. 存在しない場合は nullptr.</returns>
    Microsoft::WRL::ComPtr<IDxcBlob>* GetShaderBlob(const std::string& key);
};

} // namespace OriGine

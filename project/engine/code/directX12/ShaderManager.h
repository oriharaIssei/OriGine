#pragma once

/// stl
// memory
#include <memory>
#include <wrl.h>

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
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderCompiler.h"
#include "directX12/BlendMode.h"

const std::string shaderDirectory = "engine/resource/Shader";

class ShaderManager;
/// <summary>
/// PSOを生成するための情報をまとめたクラス
/// </summary>
class ShaderInformation {
    friend class ShaderManager;

private:
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc_{
        .DepthEnable    = true,
        .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL,
        .DepthFunc      = D3D12_COMPARISON_FUNC_LESS_EQUAL};

    std::vector<D3D12_STATIC_SAMPLER_DESC> samplerDescs_;
    std::vector<D3D12_ROOT_PARAMETER> rootParameters_;
    std::vector<std::unique_ptr<D3D12_DESCRIPTOR_RANGE[]>> descriptorRanges_;
    std::vector<D3D12_INPUT_ELEMENT_DESC> elementDescs_;

    D3D12_RASTERIZER_DESC rasterizerDesc{
        .FillMode = D3D12_FILL_MODE_SOLID,
        .CullMode = D3D12_CULL_MODE_BACK};

public:
    D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType   = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlag = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    std::string vsKey                            = ""; // Vertex Shader
    std::string psKey                            = ""; // Pixel Shader
    std::string csKey                            = ""; // Compute Shader
    std::string dsKey                            = ""; // Domain Shader
    std::string hsKey                            = ""; // Hull Shader
    std::string gsKey                            = ""; // Geometry Shader

    BlendMode blendMode_ = BlendMode::Alpha;

    /// <summary>
    /// PushBack Sampler Infomation
    /// </summary>
    /// <param name="samplerDesc"></param>
    /// <returns>Index</returns>
    size_t pushBackSamplerDesc(const D3D12_STATIC_SAMPLER_DESC& samplerDesc) {
        samplerDescs_.emplace_back(samplerDesc);
        return samplerDescs_.size() - 1;
    }
    /// <returns>Index</returns>
    size_t pushBackRootParameter(const D3D12_ROOT_PARAMETER& parameter) {
        rootParameters_.push_back(parameter);
        return rootParameters_.size() - 1;
    }
    /// <returns>Index</returns>
    size_t pushBackInputElementDesc(const D3D12_INPUT_ELEMENT_DESC& elementDesc) {
        elementDescs_.push_back(elementDesc);
        return elementDescs_.size() - 1;
    }
    void setDescriptorRange2Parameter(const D3D12_DESCRIPTOR_RANGE* range, size_t numRanges, size_t rootParameterIndex) {
        // 動的に確保して管理する
        auto ranges = std::make_unique<D3D12_DESCRIPTOR_RANGE[]>(numRanges);
        std::copy(range, range + numRanges, ranges.get());

        descriptorRanges_.push_back(std::move(ranges));
        rootParameters_[rootParameterIndex].DescriptorTable.pDescriptorRanges   = descriptorRanges_.back().get();
        rootParameters_[rootParameterIndex].DescriptorTable.NumDescriptorRanges = static_cast<UINT>(numRanges);
    }

    void changeCullMode(D3D12_CULL_MODE cullMode) {
        rasterizerDesc.CullMode = cullMode;
    }
    void changeFillMode(D3D12_FILL_MODE fillMode) {
        rasterizerDesc.FillMode = fillMode;
    }

    /// <summary>
    /// 楽だが非推奨
    /// </summary>
    /// <returns></returns>
    D3D12_DEPTH_STENCIL_DESC& customDepthStencilDesc() { return depthStencilDesc_; }
    void setDepthStencilDesc(const D3D12_DEPTH_STENCIL_DESC& desc) { depthStencilDesc_ = desc; }

    void setTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology) { topologyType = topology; }
};
using ShaderInfo = ShaderInformation;

/// <summary>
/// Shader管理クラス
/// </summary>
class ShaderManager {
public:
    static ShaderManager* getInstance() {
        static ShaderManager instance;
        return &instance;
    }

public:
    /// <summary>
    /// 初期化処理
    /// </summary>
    void Initialize();
    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize();
    /// <summary>
    /// PSOを生成する
    /// </summary>
    /// <param name="key">生成したPSOを表すKey.(すでにそのkeyが埋まっている場合はKeyの中身を返す)</param>
    PipelineStateObj* CreatePso(const std::string& key, const ShaderInformation& shaderInfo, Microsoft::WRL::ComPtr<ID3D12Device> device);
    /// <summary>
    /// Shaderを読み込んで登録する
    /// </summary>
    bool LoadShader(const std::string& fileName, const std::string& directory = shaderDirectory, const wchar_t* profile = L"vs_6_0");

private:
    ShaderManager() = default;
    ~ShaderManager() {};
    ShaderManager(const ShaderManager&)                  = delete;
    const ShaderManager& operator=(const ShaderManager&) = delete;

private:
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<IDxcBlob>> shaderBlobMap_;
    std::unique_ptr<ShaderCompiler> shaderCompiler_;
    std::unordered_map<std::string, std::unique_ptr<PipelineStateObj>> psoMap_;

public:

    ShaderCompiler* getShaderCompiler() const { return shaderCompiler_.get(); }

    /// <summary>
    /// shaderBlob を 登録されていなければ登録
    /// </summary>
    /// <param name="key"></param>
    /// <param name="shaderBlob"></param>
    /// <returns> 登録できたら true </returns>
    bool RegisterShaderBlob(const std::string& fileName, Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob);
    bool IsRegisteredShaderBlob(const std::string& fileName) const;
    void ForciblyRegisterShaderBlob(const std::string& fileName, Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob) {
        shaderBlobMap_[fileName] = std::move(shaderBlob);
    };

    bool IsRegisteredPipelineStateObj(const std::string& key) const {
        return psoMap_.find(key) != psoMap_.end();
    }
    PipelineStateObj* getPipelineStateObj(const std::string& key);
    Microsoft::WRL::ComPtr<IDxcBlob>* getShaderBlob(const std::string& key);
};

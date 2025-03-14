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
// dx12object
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderCompiler.h"

const std::string shaderDirectory = "./resource/Shader";

enum class BlendMode {
    None     = 0,
    Normal   = 1,
    Alpha    = Normal,
    Add      = 2,
    Subtract = 3,
    Sub      = Subtract,
    Multiply = 4,
    Screen   = 5,

    Count
};

constexpr size_t kBlendNum                                   = static_cast<size_t>(BlendMode::Count);
static const std::array<std::string, kBlendNum> blendModeStr = {
    "None",
    "Normal",
    "Add",
    "Subtract",
    "Multiply",
    "Screen"};

class ShaderManager;
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
    void SetDescriptorRange2Parameter(const D3D12_DESCRIPTOR_RANGE* range, size_t numRanges, size_t rootParameterIndex) {
        // 動的に確保して管理する
        auto ranges = std::make_unique<D3D12_DESCRIPTOR_RANGE[]>(numRanges);
        std::copy(range, range + numRanges, ranges.get());

        descriptorRanges_.push_back(std::move(ranges));
        rootParameters_[rootParameterIndex].DescriptorTable.pDescriptorRanges   = descriptorRanges_.back().get();
        rootParameters_[rootParameterIndex].DescriptorTable.NumDescriptorRanges = static_cast<UINT>(numRanges);
    }

    void ChangeCullMode(D3D12_CULL_MODE cullMode) {
        rasterizerDesc.CullMode = cullMode;
    }
    void ChangeFillMode(D3D12_FILL_MODE fillMode) {
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

class ShaderManager {
public:
    static ShaderManager* getInstance() {
        static ShaderManager instance;
        return &instance;
    }

public:
    void Initialize();
    void Finalize();
    PipelineStateObj* CreatePso(const std::string& key, const ShaderInformation& shaderInfo, ID3D12Device* device);

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
    bool RegisterShaderBlob(const std::string& fileName, Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob) {
        auto it = shaderBlobMap_.find(fileName);
        if (it != shaderBlobMap_.end()) {
            return false;
        }
        shaderBlobMap_.emplace(fileName, std::move(shaderBlob));
        return true;
    };
    void ForciblyRegisterShaderBlob(const std::string& fileName, Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob) {
        shaderBlobMap_[fileName] = std::move(shaderBlob);
    };

    bool LoadShader(const std::string& fileName, const std::string& directory = shaderDirectory, const wchar_t* profile = L"vs_6_0");

    PipelineStateObj* getPipelineStateObj(const std::string& key) { return psoMap_[key].get(); }
    const Microsoft::WRL::ComPtr<IDxcBlob>& getShaderBlob(const std::string& key) {
        auto it = shaderBlobMap_.find(key);
        if (it == shaderBlobMap_.end()) {
            throw std::runtime_error("Shader blob not found for key: " + key);
        }
        return it->second;
    };
};

#include "directX12/ShaderManager.h"

/// api
#include <Windows.h>

/// engine
// asset
#include "asset/AssetSystem.h"
#include "asset/manager/ShaderAssetManager.h"

/// assert (log)
#include "logger/Logger.h"
#include <cassert>

/// util
#include "util/StringUtil.h"

using namespace OriGine;

ShaderManager* ShaderManager::GetInstance() {
    static ShaderManager instance;
    return &instance;
}

AssetManager<ShaderAsset>* ShaderManager::GetShaderAssetManager() {
    return AssetSystem::GetInstance()->GetManager<ShaderAsset>();
}

void ShaderManager::Initialize() {
    // シェーダーのコンパイラ本体は ShaderAssetManager 配下の ShaderLoader が保持するため、
    // ここでは PSO 用のキャッシュを空にするだけでよい
    psoMap_.clear();
    shaderKeyToAssetIndex_.clear();
}

void ShaderManager::Finalize() {
    // PSO解放
    for (auto& pso : psoMap_) {
        pso.second->Finalize();
        pso.second.reset();
    }
    psoMap_.clear();

    // 参照していたシェーダーアセットを解放する。
    // AssetSystem::Finalize が先に走っているとマネージャは既に居ないので、
    // その場合はインデックスを捨てるだけでよい
    if (auto* manager = GetShaderAssetManager()) {
        for (const auto& [key, assetIndex] : shaderKeyToAssetIndex_) {
            manager->ReleaseAsset(assetIndex);
        }
    }
    shaderKeyToAssetIndex_.clear();
}

PipelineStateObj* ShaderManager::CreatePso(const std::string& _key,
    const ShaderInformation& _shaderInfo,
    Microsoft::WRL::ComPtr<ID3D12Device> _device) {

    // すでに存在するならそれを返す
    auto it = psoMap_.find(_key);
    if (it != psoMap_.end()) {
        return it->second.get();
    }

    std::unique_ptr<PipelineStateObj> pso;
    pso = std::make_unique<PipelineStateObj>();
    HRESULT result;

    ///=================================================
    /// RootSignature
    ///=================================================
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {
        .NumParameters     = static_cast<uint32_t>(_shaderInfo.rootParameters_.size()),
        .pParameters       = _shaderInfo.rootParameters_.data(),
        .NumStaticSamplers = static_cast<uint32_t>(_shaderInfo.samplerDescs_.size()),
        .pStaticSamplers   = _shaderInfo.samplerDescs_.data(),
        .Flags             = _shaderInfo.rootSignatureFlag,
    };

    // シリアライズしてバイナリにする
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob     = nullptr;

    result = D3D12SerializeRootSignature(
        &rootSignatureDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &signatureBlob,
        &errorBlob);

    // エラー処理
    if (FAILED(result)) {
        std::string errMsg(
            static_cast<const char*>(errorBlob->GetBufferPointer()),
            errorBlob->GetBufferSize());
        LOG_ERROR("D3D12SerializeRootSignature failed: {}", errMsg);

        OutputDebugStringA(("D3D12SerializeRootSignature failed: " + errMsg + "\n").c_str());

        assert(false);
    }

    // RootSignatureの生成
    _device->CreateRootSignature(
        0,
        signatureBlob->GetBufferPointer(),
        signatureBlob->GetBufferSize(),
        IID_PPV_ARGS(&pso->rootSignature));

    ///=================================================
    /// InputLayout
    ///=================================================
    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{
        .pInputElementDescs = _shaderInfo.elementDescs_.data(),
        .NumElements        = static_cast<uint32_t>(_shaderInfo.elementDescs_.size()),
    };

    ///=================================================
    /// BlendDesc 初期化
    ///=================================================
    D3D12_BLEND_DESC blendDesc{};
    blendDesc.RenderTarget[0] = {}; // RenderTarget[0] を初期化

    blendDesc = CreateBlendDescByBlendMode(_shaderInfo.blendMode_);

    ///=================================================
    /// GRAPHICS_PIPELINE_STATE_DESC 初期化
    ///=================================================
    // cs か graphics pipeline state かを判定
    if (!_shaderInfo.csKey.empty()) {
        IDxcBlob* csBlob = GetShaderBlob(_shaderInfo.csKey);
        if (csBlob == nullptr) {
            LOG_ERROR("Shader blob not found: {} (pso: {})", _shaderInfo.csKey, _key);
            return nullptr;
        }

        D3D12_COMPUTE_PIPELINE_STATE_DESC computeStateDesc{};
        computeStateDesc.CS             = {csBlob->GetBufferPointer(), csBlob->GetBufferSize()};
        computeStateDesc.pRootSignature = pso->rootSignature.Get();

        result = _device->CreateComputePipelineState(
            &computeStateDesc,
            IID_PPV_ARGS(&pso->pipelineState));

        if (FAILED(result)) {
            std::string resultString = HrToString(result);
            LOG_ERROR("CreateComputePipelineState failed: {}", resultString);

            OutputDebugStringA(("CreateComputePipelineState failed:\n" + resultString + "\n").c_str());

            assert(false);
        }

        psoMap_[_key] = std::move(pso);
        LOG_INFO("Compute Pipeline State Object created: {}", _key);

        return psoMap_[_key].get();
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc{};

    // 各ステージのバイナリを引き当てる。
    // キーが指定されているのに未読み込みなら PSO は組めないので、その場で打ち切る
    // （以前は未読み込みでもマップに空要素を作って参照し、nullptr 参照で落ちていた）
    bool hasMissingShader = false;
    auto bindStage        = [&](const std::string& _stageKey, D3D12_SHADER_BYTECODE& _outByteCode) {
        if (_stageKey.empty()) {
            return;
        }
        IDxcBlob* blob = GetShaderBlob(_stageKey);
        if (blob == nullptr) {
            LOG_ERROR("Shader blob not found: {} (pso: {})", _stageKey, _key);
            hasMissingShader = true;
            return;
        }
        _outByteCode = {blob->GetBufferPointer(), blob->GetBufferSize()};
    };

    bindStage(_shaderInfo.vsKey, pipelineStateDesc.VS);
    bindStage(_shaderInfo.psKey, pipelineStateDesc.PS);
    bindStage(_shaderInfo.dsKey, pipelineStateDesc.DS);
    bindStage(_shaderInfo.hsKey, pipelineStateDesc.HS);
    bindStage(_shaderInfo.gsKey, pipelineStateDesc.GS);

    if (hasMissingShader) {
        return nullptr;
    }

    pipelineStateDesc.pRootSignature = pso->rootSignature.Get();
    pipelineStateDesc.InputLayout    = inputLayoutDesc;

    pipelineStateDesc.BlendState      = blendDesc;
    pipelineStateDesc.RasterizerState = _shaderInfo.rasterizerDesc;

    pipelineStateDesc.DepthStencilState = _shaderInfo.depthStencilDesc_;
    pipelineStateDesc.DSVFormat         = DXGI_FORMAT_D24_UNORM_S8_UINT;

    pipelineStateDesc.NumRenderTargets = 1;
    pipelineStateDesc.RTVFormats[0]    = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    // 利用するトポロジ(形状)タイプ。
    pipelineStateDesc.PrimitiveTopologyType = _shaderInfo.topologyType;
    // どのように画面に色を打ち込むかの設定
    pipelineStateDesc.SampleDesc.Count = 1;
    pipelineStateDesc.SampleMask       = D3D12_DEFAULT_SAMPLE_MASK;

    result = _device->CreateGraphicsPipelineState(
        &pipelineStateDesc,
        IID_PPV_ARGS(&pso->pipelineState));

    if (FAILED(result)) {
        std::string resultString = HrToString(result);
        LOG_ERROR("CreateGraphicsPipelineState failed: {}", resultString);
        OutputDebugStringA(("CreateGraphicsPipelineState" + resultString + "\n").c_str());
        assert(SUCCEEDED(result));
    };

    psoMap_[_key] = std::move(pso);

    return psoMap_[_key].get();
}

bool ShaderManager::LoadShader(const std::string& _fileName, const std::string& _directory, const wchar_t* _profile) {
    // 同じキーで既に読み込み済みなら何もしない（従来どおり false を返す）
    if (shaderKeyToAssetIndex_.find(_fileName) != shaderKeyToAssetIndex_.end()) {
        return false;
    }

    auto* manager = GetShaderAssetManager();
    if (manager == nullptr) {
        LOG_ERROR("ShaderAssetManager is not registered. (shader: {})", _fileName);
        return false;
    }

    // プロファイルはバリアントとして渡す。
    // 同じ HLSL でもプロファイルが違えば別バイナリになるため、
    // AssetManager 側では「パス + プロファイル」でキャッシュされる
    const std::string assetPath = _directory + '/' + _fileName + ".hlsl";
    const std::string profile   = ConvertString(std::wstring(_profile));

    const size_t assetIndex = manager->LoadAsset(assetPath, profile);
    if (!manager->IsAlive(assetIndex)) {
        LOG_ERROR("Failed to load shader: {} (profile: {})", assetPath, profile);
        return false;
    }
    if (manager->GetAsset(assetIndex).blob == nullptr) {
        // スロットは確保できたがコンパイルに失敗しているケース。
        // キーを登録すると壊れたアセットを掴み続けることになるので、参照を返して打ち切る
        LOG_ERROR("Failed to compile shader: {} (profile: {})", assetPath, profile);
        manager->ReleaseAsset(assetIndex);
        return false;
    }

    shaderKeyToAssetIndex_.emplace(_fileName, assetIndex);
    return true;
}

PipelineStateObj* ShaderManager::GetPipelineStateObj(const std::string& _key) {
    auto it = psoMap_.find(_key);
    if (it == psoMap_.end()) {
        return nullptr;
    }
    return it->second.get();
}

IDxcBlob* ShaderManager::GetShaderBlob(const std::string& _key) const {
    auto it = shaderKeyToAssetIndex_.find(_key);
    if (it == shaderKeyToAssetIndex_.end()) {
        return nullptr;
    }
    auto* manager = GetShaderAssetManager();
    if (manager == nullptr || !manager->IsAlive(it->second)) {
        return nullptr;
    }
    return manager->GetAsset(it->second).blob.Get();
}

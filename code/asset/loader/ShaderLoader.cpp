#include "ShaderLoader.h"

/// stl
#include <algorithm>
#include <cctype>
#include <filesystem>

/// util
#include "logger/Logger.h"
#include "util/StringUtil.h"

using namespace OriGine;

void ShaderLoader::Initialize() {
    shaderCompiler_ = std::make_unique<ShaderCompiler>();
    shaderCompiler_->Initialize();
}

void ShaderLoader::Finalize() {
    if (shaderCompiler_) {
        shaderCompiler_->Finalize();
        shaderCompiler_.reset();
    }
}

/// <summary>
/// ファイル名からプロファイルを推測する.
/// シェーダー資産の命名規約が "&lt;Name&gt;.&lt;Stage&gt;.hlsl" で統一されているため、
/// 拡張子を1つ剥がした結果の末尾をステージ名として解釈する.
/// シェーダーモデルのバージョンまでは名前から決められないので、
/// 既定の 6_0 を採用する（6_5 等が必要な場合は呼び出し側でプロファイルを明示する）.
/// </summary>
std::string ShaderLoader::GuessProfile(const std::string& _assetPath) {
    // "path/to/Object3dTexture.VS.hlsl" -> "Object3dTexture.VS" -> "VS"
    std::filesystem::path path(_assetPath);
    const std::string stem = path.stem().string();

    const size_t dotPos = stem.rfind('.');
    if (dotPos == std::string::npos) {
        LOG_WARN("Could not determine shader stage from '{}'. Fallback to vs_6_0.", _assetPath);
        return "vs_6_0";
    }

    std::string stage = stem.substr(dotPos + 1);
    std::transform(stage.begin(), stage.end(), stage.begin(),
        [](unsigned char _c) { return static_cast<char>(std::tolower(_c)); });

    if (stage == "vs" || stage == "ps" || stage == "cs" || stage == "ds" || stage == "hs" || stage == "gs" || stage == "ms" || stage == "as" || stage == "lib") {
        return stage + "_6_0";
    }

    LOG_WARN("Unknown shader stage '{}' in '{}'. Fallback to vs_6_0.", stage, _assetPath);
    return "vs_6_0";
}

ShaderAsset ShaderLoader::LoadAsset(const std::string& _assetPath) {
    return LoadAsset(_assetPath, std::string{});
}

ShaderAsset ShaderLoader::LoadAsset(const std::string& _assetPath, const std::string& _variant) {
    ShaderAsset asset;

    if (!shaderCompiler_) {
        LOG_ERROR("ShaderCompiler is not initialized. (path: {})", _assetPath);
        return asset;
    }

    // バリアント（プロファイル）が指定されていなければ、ファイル名から推測する
    asset.profile = _variant.empty() ? GuessProfile(_assetPath) : _variant;

    IDxcBlob* blob = shaderCompiler_->CompileShader(
        ConvertString(_assetPath),
        ConvertString(asset.profile).c_str());

    if (blob == nullptr) {
        LOG_ERROR("Failed to compile shader: {} (profile: {})", _assetPath, asset.profile);
        return asset;
    }

    // CompileShader は AddRef 済みの生ポインタを返すため、
    // ComPtr へは Attach で所有権ごと移す（コピー代入だと参照カウントが1多く残る）
    asset.blob.Attach(blob);

    LOG_TRACE("Shader compiled: {} (profile: {})", _assetPath, asset.profile);

    return asset;
}

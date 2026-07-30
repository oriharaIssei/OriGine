#pragma once

#include "asset/ShaderAsset.h"
#include "IAssetLoader.h"

/// stl
#include <memory>

/// engine
// directX12
#include "directX12/ShaderCompiler.h"

namespace OriGine {

/// <summary>
/// HLSL ソースを DXC でコンパイルしてシェーダーバイナリを生成するローダー.
/// バリアントとしてプロファイル (vs_6_0 / ps_6_5 / cs_6_0 など) を受け取る.
/// </summary>
class ShaderLoader
    : public IAssetLoader<ShaderAsset> {
public:
    ShaderLoader()           = default;
    ~ShaderLoader() override = default;

    void Initialize() override;
    void Finalize() override;

    /// <summary>
    /// プロファイル指定なしで読み込む.
    /// ファイル名の末尾（.VS / .PS / .CS ...）からプロファイルを推測する.
    /// </summary>
    ShaderAsset LoadAsset(const std::string& _assetPath) override;

    /// <summary>
    /// プロファイルを指定して読み込む.
    /// </summary>
    /// <param name="_assetPath">HLSL ファイルのパス</param>
    /// <param name="_variant">プロファイル文字列 (例: "vs_6_0")。空ならファイル名から推測する</param>
    ShaderAsset LoadAsset(const std::string& _assetPath, const std::string& _variant) override;

    /// <summary>
    /// ファイル名からプロファイルを推測する.
    /// "Object3dTexture.VS.hlsl" のように、拡張子を除いた末尾がステージ名になっている前提.
    /// </summary>
    /// <param name="_assetPath">HLSL ファイルのパス</param>
    /// <returns>推測したプロファイル。判別できない場合は "vs_6_0"</returns>
    static std::string GuessProfile(const std::string& _assetPath);

    /// <summary>シェーダーコンパイラへのポインタを取得する.</summary>
    ShaderCompiler* GetShaderCompiler() const { return shaderCompiler_.get(); }

private:
    std::unique_ptr<ShaderCompiler> shaderCompiler_;
};

} // namespace OriGine

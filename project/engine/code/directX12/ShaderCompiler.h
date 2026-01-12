#pragma once

/// Microsoft
#include <wrl.h>

#include <dxcapi.h>

/// stl
#include <string>

namespace OriGine {

/// <summary>
/// DirectX Shader Compiler (DXC) を使用して、HLSL ソースコードをコンパイルし、
/// GPU が実行可能なバイナリ (Blob) を生成するクラス.
/// </summary>
class ShaderCompiler {
public:
    /// <summary>
    /// DXC の各インターフェース (Utils, Compiler, IncludeHandler) を初期化する.
    /// </summary>
    void Initialize();

    /// <summary>
    /// ファイルパスとプロファイルを指定して HLSL シェーダーをコンパイルする.
    /// </summary>
    /// <param name="filePath">コンパイル対象の HLSL ファイルへのパス</param>
    /// <param name="profile">ターゲットプロファイル (例: L"vs_6_0", L"ps_6_0")</param>
    /// <returns>コンパイル結果のバイナリデータ (IDxcBlob). 失敗時は nullptr を返す場合がある.</returns>
    IDxcBlob* CompileShader(const std::wstring& filePath, const wchar_t* profile);

    /// <summary>
    /// 保持している DXC 関連オブジェクトを解放する.
    /// </summary>
    void Finalize();

private:
    /// <summary>DXC ユーティリティインターフェース</summary>
    Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_ = nullptr;
    /// <summary>DXC コンパイラインターフェース</summary>
    Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_ = nullptr;
    /// <summary>シェーダー内の #include を処理するハンドラ</summary>
    Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_ = nullptr;

private:
    /// <summary>
    /// 指定されたシェーダーファイルをディスクから読み込み、DXC 内部形式のバッファへ格納する.
    /// </summary>
    /// <param name="filePath">HLSL ファイルのパス</param>
    /// <param name="shaderSource">読み込んだデータの格納先 (BlobEncoding)</param>
    /// <param name="buf">DXC で使用するバッファ記述構造体</param>
    void LoadShaderFile(const std::wstring& filePath, IDxcBlobEncoding* shaderSource, DxcBuffer& buf);
};

} // namespace OriGine

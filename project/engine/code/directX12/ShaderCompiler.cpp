#include "ShaderCompiler.h"

/// Microsoft
#include <Windows.h>

/// stl
#include <cassert>
/// util
#include "logger/Logger.h"
#include "util/StringUtil.h"

#pragma comment(lib, "dxcompiler.lib")

using namespace OriGine;

void ShaderCompiler::Initialize() {
    // DxCの各種インスタンスを生成する
    HRESULT hr;
    hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
    assert(SUCCEEDED(hr));

    hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));
    assert(SUCCEEDED(hr));

    hr = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
    assert(SUCCEEDED(hr));
}

IDxcBlob* ShaderCompiler::CompileShader(const std::wstring& filePath, const wchar_t* profile) {
    LOG_DEBUG("Begin CompileShader, path : {}, profile : {}\n", ConvertString(filePath), ConvertString(profile));

    HRESULT hr;

    IDxcBlobEncoding* shaderSource = nullptr;
    DxcBuffer buf;
    LoadShaderFile(filePath, shaderSource, buf);

    /*---------- Setting Compile Option ----------*/
    LPCWSTR args[] = {
        filePath.c_str(), // コンパイル対象のパス
        L"-E", L"main", // エントリーポイントの指名。(基本main)
        L"-T", profile, // ShaderProfileの設定
        L"-Zi", L"-Qembed_debug", // デバッグ用の情報を埋め込む
        L"-Od", // 最適化を外す
        L"-Zpr", // メモリレイアウトは行優先
    };
    /*--------------------------------------------*/

    ///=============================================
    /// 実際にコンパイルする
    ///=============================================
    IDxcResult* shaderResult = nullptr;
    hr                       = dxcCompiler_->Compile(
        &buf,
        args,
        _countof(args),
        includeHandler_.Get(),
        IID_PPV_ARGS(&shaderResult));
    assert(SUCCEEDED(hr));

    ///=============================================
    /// 警告・エラーチェック
    ///=============================================
    IDxcBlobUtf8* shaderError = nullptr;
    shaderResult->GetOutput(
        DXC_OUT_ERRORS,
        IID_PPV_ARGS(&shaderError),
        nullptr);

    if (shaderError != nullptr
        && shaderError->GetStringLength() != 0) {
        LOG_ERROR("{}", shaderError->GetStringPointer());
        assert(false);
    }

    ///=============================================
    /// コンパイル結果の受け取り
    ///=============================================
    IDxcBlob* shaderBlob = nullptr;
    hr                   = shaderResult->GetOutput(
        DXC_OUT_OBJECT,
        IID_PPV_ARGS(&shaderBlob),
        nullptr);
    assert(SUCCEEDED(hr));
    LOG_DEBUG("Compile Succeeded, path : {}, profile : {}\n", ConvertString(filePath), ConvertString(profile));

    return shaderBlob;
}

void ShaderCompiler::Finalize() {
    dxcUtils_.Reset();
    dxcCompiler_.Reset();
    includeHandler_.Reset();
}

void ShaderCompiler::LoadShaderFile(const std::wstring& filePath, IDxcBlobEncoding* shaderSource, DxcBuffer& buf) {
    // hlslファイルを読み込む
    HRESULT hr = dxcUtils_->LoadFile(
        filePath.c_str(),
        nullptr,
        &shaderSource);

    if (FAILED(hr)) {
        LOG_ERROR("Failed to load shader file: {}, HRESULT: {}", ConvertString(filePath), HrToString(hr));
        assert(false);
        return;
    }

    // 読み込んだファイルの内容を設定する
    buf.Ptr      = shaderSource->GetBufferPointer();
    buf.Size     = shaderSource->GetBufferSize();
    buf.Encoding = DXC_CP_UTF8;
}

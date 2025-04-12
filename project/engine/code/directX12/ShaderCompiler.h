#pragma once

#include <string>
#include <wrl.h>

#include <dxcapi.h>

class ShaderCompiler {
public:
	void Initialize();
	/// <summary>
	/// DxCを使ってShaderをCompileする
	/// </summary>
	/// <param name="filePath">CompileするShaderファイルへのパス</param>
	/// <param name="profile">CompilerにしようするProfile</param>
	/// <returns></returns>
	IDxcBlob* CompileShader(const std::wstring& filePath,const wchar_t* profile);
	void Finalize();
private:
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_ = nullptr;
	Microsoft::WRL::ComPtr <IDxcCompiler3> dxcCompiler_ = nullptr;
	Microsoft::WRL::ComPtr <IDxcIncludeHandler> includeHandler_ = nullptr;
private:
	/// <summary>
	/// Shaderを読み込んでDxcBufferに内容を保存する
	/// </summary>
	/// <param name="filePath">shaderファイルのPath</param>
	/// <param name="shaderSource">読み込んだ内容を保存する(実質的な引数)</param>
	/// <param name="buf">内容の設定をする(実質的な引数)</param>
	/// <returns></returns>
	void LoadShaderFile(const std::wstring& filePath,IDxcBlobEncoding* shaderSource, DxcBuffer& buf);
};

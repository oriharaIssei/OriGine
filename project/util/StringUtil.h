#pragma once

/// stl
#include <format>
#include <string>

/// <summary>
/// widestring と string の相互変換
/// </summary>
/// <param name="_str">変換前</param>
/// <returns>変換後</returns>
std::wstring ConvertString(const std::string& _str);
/// <summary>
/// widestring と string の相互変換
/// </summary>
/// <param name="_str">変換前</param>
/// <returns>変換後</returns>
std::string ConvertString(const std::wstring& _str);

/// <summary>
/// HRESULTコードを文字列に変換する
/// </summary>
typedef long HRESULT;
std::string HrToString(HRESULT _hr);

/// <summary>
/// パス文字列の正規化 ( '\\', を '/' に変換 )
/// </summary>
/// <param name="_path"></param>
/// <returns></returns>
std::string NormalizeString(const std::string& _path);

/// <summary>
/// 現在日時を "YYYY-MM-DD_HH-MM-SS" 形式で取得する
/// </summary>
/// <returns></returns>
std::string TimeToString();

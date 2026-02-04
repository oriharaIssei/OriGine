#pragma once

/// stl
#include <algorithm>
#include <format>
#include <sstream>
#include <string>
#include <vector>

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

/// <summary>
/// 文字列を区切り文字で分割する
/// </summary>
inline std::vector<std::string> Split(const std::string& _str, char _delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(_str);
    std::string item;
    while (std::getline(ss, item, _delimiter)) {
        result.push_back(item);
    }
    return result;
}

/// <summary>
/// 文字列の前後の空白を削除する
/// </summary>
inline std::string Trim(const std::string& _str) {
    auto start = std::find_if_not(_str.begin(), _str.end(), [](unsigned char c) { return std::isspace(c); });
    auto end   = std::find_if_not(_str.rbegin(), _str.rend(), [](unsigned char c) { return std::isspace(c); }).base();
    return (start < end) ? std::string(start, end) : std::string();
}

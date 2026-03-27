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
/// <param name="str">変換前</param>
/// <returns>変換後</returns>
std::wstring ConvertString(const std::string& str);
/// <summary>
/// widestring と string の相互変換
/// </summary>
/// <param name="str">変換前</param>
/// <returns>変換後</returns>
std::string ConvertString(const std::wstring& str);

/// <summary>
/// HRESULTコードを文字列に変換する
/// </summary>
typedef long HRESULT;
std::string HrToString(HRESULT hr);

/// <summary>
/// パス文字列の正規化 ( '\\', を '/' に変換 )
/// </summary>
/// <param name="path"></param>
/// <returns></returns>
std::string NormalizeString(const std::string& path);

/// <summary>
/// 現在日時を "YYYY-MM-DD_HH-MM-SS" 形式で取得する
/// </summary>
/// <returns></returns>
std::string TimeToString();

/// <summary>
/// 文字列を区切り文字で分割する
/// </summary>
/// <param name="str">分割する文字列</param>
/// <param name="delimiter">区切り文字</param>
std::vector<std::string> Split(const std::string& str, char delimiter = ' ');

/// <summary>
/// 文字列の前後の空白を削除する
/// </summary>
/// <param name="str">トリムする文字列</param>
std::string Trim(const std::string& str);

/// <summary>
/// 最初の改行文字以降を削除する (\r\n 両対応)
/// </summary>
/// <param name="str">処理する文字列</param>
/// <param name="includeNewline">true : 改行文字を結果に含める / false : 改行文字も削除する</param>
std::string TrimAfterNewline(const std::string& str, bool includeNewline = false);

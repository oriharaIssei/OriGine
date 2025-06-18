#pragma once

#include <format>
#include <string>

std::wstring ConvertString(const std::string& str);
std::string ConvertString(const std::wstring& str);

typedef long HRESULT;
std::string HrToString(HRESULT hr);

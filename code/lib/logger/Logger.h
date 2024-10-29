#pragma once
#include <string>
#include <format>

/*
	ConvertStringはまた別のクラスを作るべきなのでは？
	(他の場所でも使うから)
*/

class Logger {
public:
	static void OutputLog(const std::string& str);
	static void OutputLog(const std::wstring& str);

	static std::wstring ConvertString(const std::string& str);
	static std::string ConvertString(const std::wstring& str);
};
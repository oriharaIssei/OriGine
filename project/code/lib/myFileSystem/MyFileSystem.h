#pragma once

#include <filesystem>
#include <list>
#include <string>

class MyFileSystem {
public:
    /// <summary>
    /// 特定のファイルフォーマットを検索
    /// </summary>
    /// <param name="directory">検索するフォルダーのパス</param>
    /// <param name="extension">fileFormat (.pngとか)</param>
    /// <returns>first = directory, second = fileName </returns>
    static std::list<std::pair<std::string, std::string>> SearchFile(const std::string& directory, const std::string& extension, bool withoutExtensionOutput = true);

    static std::list<std::string> SearchSubFolder(const std::string& directory);

    static bool CreateFolder(const std::string& directory);

    static void SelectFolderDialog(const std::string& _defaultDirectory, std::string& _outPath);
    static void SelectFileDialog(const std::string& _defaultDirectory, std::string& _outPath);

    static bool removeEmptyFolder(const std::string& directory);
    static std::uintmax_t deleteFolder(const std::string& path);
    static std::uintmax_t deleteFile(const std::string& filePath);
};

using myfs = MyFileSystem;
using myFs = MyFileSystem;

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
    static std::list<std::pair<std::string, std::string>> searchFile(const std::string& directory, const std::string& extension, bool withoutExtensionOutput = true);

    static std::list<std::string> searchSubFolder(const std::string& directory);

    static bool createFile(const std::string& filePath);
    static bool createFolder(const std::string& directory);

    static void selectFolderDialog(const std::string& _defaultDirectory, std::string& _outPath);
    static bool selectFileDialog(
        const std::string& defaultDirectory, std::string& fileDirectory, std::string& filename, const std::vector<std::string>& extensions, bool withoutExtensionOutput = false);

    static bool removeEmptyFolder(const std::string& directory);
    static std::uintmax_t deleteFolder(const std::string& path);
    static std::uintmax_t deleteFile(const std::string& filePath);
};

using myfs = MyFileSystem;
using myFs = MyFileSystem;

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
    static bool selectFileDialog(const std::string& defaultDirectory, std::string& fileDirectory, std::string& filename, const std::vector<std::string>& extensions);

    static bool removeEmptyFolder(const std::string& directory);
    static std::uintmax_t deleteFolder(const std::string& path);
    static std::uintmax_t deleteFile(const std::string& filePath);

#ifdef _DEBUG
    /// <summary>
    /// GUIのファイル選択ダイアログを表示
    /// </summary>
    /// <param name="_defaultDirectory">初期表示するフォルダ</param>
    /// <param name="_fileDirectory">選択したフォルダ(_defaultDirectoryから見た directory)</param>
    /// <param name="_filename">選択したファイル名</param>
    /// <param name="_extensions">拡張子のリスト</param>
    /// <returns>選択したかどうか</returns>
    /// <remarks>拡張子は「.png」などの形式で指定</remarks>
    // static bool selectFileGuiDialog(
    //     const std::string& _defaultDirectory,
    //     std::string& _fileDirectory,
    //     std::string& _filename,
    //     const std::vector<std::string>& _extensions);

    /// <summary>
    /// GUIのフォルダ選択ダイアログを表示
    /// </summary>
    /// <param name="_defaultDirectory">初期表示するフォルダ</param>
    /// <param name="_outPath">選択したフォルダ(_defaultDirectoryから見た directory)</param>
    /// <returns>選択したかどうか</returns>
    // static bool selectFolderGuiDialog(const std::string& _defaultDirectory, std::string& _outPath);

#endif // _DEBUG
};

using myfs = MyFileSystem;
using myFs = MyFileSystem;

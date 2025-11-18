#pragma once

/// stl
// container
#include <list>
// string
#include <filesystem>
#include <string>
// thread
#include <atomic>
#include <thread>

/// <summary>
/// std::filesystemを自分用にわかりやすくラップしたクラス
/// </summary>
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

/// <summary>
/// ポーリングによる ファイル監視クラス
/// </summary>
class FileWatcher {
public:
    FileWatcher(const std::string& _filePath, int32_t _intervalMs = 1000);
    ~FileWatcher();

    /// <summary>
    /// 監視を開始する
    /// </summary>
    void Start();
    /// <summary>
    /// 監視を停止する
    /// </summary>
    void Stop();

private:
    /// <summary>
    /// 監視処理ループ
    /// </summary>
    void watchLoop();

private:
    std::string filePath_;
    int32_t intervalMs_;
    std::atomic<bool> isChanged_ = false;
    std::atomic<bool> isRunning_ = false;
    std::thread watcherThread_;
    std::filesystem::file_time_type lastWriteTime_;

public:
    const std::string& GetFilePath() const { return filePath_; }
    void SetFilePath(const std::string& _filePath) {
        filePath_      = _filePath;
        lastWriteTime_ = std::filesystem::last_write_time(filePath_);
    }
    bool isChanged() {
        bool changed = isChanged_;
        if (changed) {
            isChanged_ = false; // 一度チェックしたらフラグをリセット
        }
        return changed;
    }
};

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
    static std::list<std::pair<std::string, std::string>> SearchFile(const std::string& _directory, const std::string& _extension, bool _withoutExtensionOutput = true);

    /// <summary>
    /// サブフォルダーを検索
    /// </summary>
    /// <param name="directory"></param>
    /// <returns></returns>
    static std::list<std::string> SearchSubFolder(const std::string& _directory);

    /// <summary>
    /// ファイルを作成する
    /// </summary>
    /// <param name="filePath"></param>
    /// <returns></returns>
    static bool CreateMyFile(const std::string& _filePath);
    /// <summary>
    /// フォルダーを作成する
    /// </summary>
    /// <param name="directory"></param>
    /// <returns></returns>
    static bool CreateFolder(const std::string& _directory);

    /// <summary>
    /// フォルダ選択ダイアログを表示する
    /// </summary>
    /// <param name="_defaultDirectory"></param>
    /// <param name="_outPath"></param>
    static void SelectFolderDialog(const std::string& _defaultDirectory, std::string& _outPath);
    /// <summary>
    /// ファイル選択ダイアログを表示する
    /// </summary>
    /// <param name="defaultDirectory"></param>
    /// <param name="fileDirectory"></param>
    /// <param name="filename"></param>
    /// <param name="extensions"></param>
    /// <param name="withoutExtensionOutput"></param>
    /// <returns></returns>
    static bool SelectFileDialog(
        const std::string& _defaultDirectory, std::string& _fileDirectory, std::string& _filename, const std::vector<std::string>& _extensions, bool _withoutExtensionOutput = false);

    /// <summary>
    /// 空のフォルダーを削除する
    /// </summary>
    /// <param name="directory"></param>
    /// <returns></returns>
    static bool RemoveEmptyFolder(const std::string& _directory);
    /// <summary>
    /// フォルダーを削除する
    /// </summary>
    /// <param name="path"></param>
    /// <returns></returns>
    static std::uintmax_t DeleteFolder(const std::string& _path);
    /// <summary>
    /// ファイルを削除する
    /// </summary>
    /// <param name="filePath"></param>
    /// <returns></returns>
    static std::uintmax_t DeleteMyFile(const std::string& _filePath);
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
    /// <summary>
    /// 監視対象ファイルパスを取得
    /// </summary>
    const std::string& GetFilePath() const { return filePath_; }
    /// <summary>
    /// 監視対象ファイルパスを設定
    /// </summary>
    /// <param name="_filePath">ファイルパス</param>
    void SetFilePath(const std::string& _filePath) {
        filePath_      = _filePath;
        lastWriteTime_ = std::filesystem::last_write_time(filePath_);
    }
    /// <summary>
    /// ファイルが更新されたか確認し、フラグをリセットする
    /// </summary>
    /// <returns>更新されていればtrue</returns>
    bool isChanged() {
        bool changed = isChanged_;
        if (changed) {
            isChanged_ = false; // 一度チェックしたらフラグをリセット
        }
        return changed;
    }
};

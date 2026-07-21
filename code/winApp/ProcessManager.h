#pragma once

#include <Windows.h>

#include <cstdint>
#include <string>
#include <vector>

namespace OriGine {

/// <summary>
/// 他プロセスが所有するウィンドウ1つ分の情報を保持する構造体.
/// ウィンドウ列挙やフォアグラウンドウィンドウの問い合わせ結果として使用される.
/// </summary>
struct WindowProcessInfo {
    DWORD processId = 0; // ウィンドウを所有するプロセスID
    HWND hwnd = nullptr; // ウィンドウハンドル
    std::wstring exeName; // 実行ファイル名（フルパスの末尾部分のみ）
    std::wstring windowTitle; // ウィンドウタイトル文字列
    bool isForeground = false; // フォアグラウンド（アクティブ）ウィンドウかどうか
};

/// <summary>
/// 他プロセスのウィンドウ列挙・操作（前面化・終了要求・強制終了・起動）を行う静的ユーティリティクラス.
/// </summary>
class ProcessManager {
public:
    /// <summary>
    /// 現在デスクトップ上に存在する可視ウィンドウを列挙する.
    /// 子ウィンドウやツールウィンドウは対象外.
    /// </summary>
    /// <returns>列挙されたウィンドウ情報のリスト</returns>
    static std::vector<WindowProcessInfo> EnumerateWindows();
    /// <summary>
    /// 現在フォアグラウンドにあるウィンドウの情報を取得する.
    /// </summary>
    /// <returns>フォアグラウンドウィンドウの情報</returns>
    static WindowProcessInfo GetForegroundApp();

    /// <summary>
    /// 指定したパスの実行ファイル・ドキュメントを起動する.
    /// </summary>
    /// <param name="path">実行ファイルのパス</param>
    /// <param name="args">起動時に渡すコマンドライン引数</param>
    /// <returns>起動に成功したか</returns>
    static bool LaunchProcess(const std::wstring& path, const std::wstring& args = L"");
    /// <summary>
    /// 指定したウィンドウを最前面（フォアグラウンド）へ表示する.
    /// </summary>
    /// <param name="hwnd">対象ウィンドウハンドル</param>
    /// <returns>操作に成功したか</returns>
    static bool BringToForeground(HWND hwnd);
    /// <summary>
    /// 指定したウィンドウへ終了メッセージ（WM_CLOSE）を送信する.
    /// </summary>
    /// <param name="hwnd">対象ウィンドウハンドル</param>
    /// <returns>メッセージ送信に成功したか</returns>
    static bool CloseWindow(HWND hwnd);
    /// <summary>
    /// 指定したプロセスIDのプロセスを強制終了する.
    /// </summary>
    /// <param name="processId">対象プロセスID</param>
    /// <returns>終了に成功したか</returns>
    static bool TerminateApp(DWORD processId);

    /// <summary>
    /// ウィンドウ情報のリストを、人が読める形式のテキストへ整形する.
    /// </summary>
    /// <param name="windows">整形対象のウィンドウ情報リスト</param>
    /// <returns>整形されたテキスト</returns>
    static std::string FormatAsText(const std::vector<WindowProcessInfo>& windows);
};

} // namespace OriGine

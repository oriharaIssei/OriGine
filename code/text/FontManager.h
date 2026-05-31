#pragma once

#include "text/BitmapFont.h"
#include "text/FontTypes.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace OriGine {

/// <summary>
/// フォントリソースのロードと一元管理を行うシングルトンクラス。
/// FontHandle をキーに複数の BitmapFont を保持し、TextComponent から
/// 使用するフォントを選択できるようにする。
/// 登録するフォント一覧はハードコードせず JSON 設定から読み込む。
/// </summary>
class FontManager {
public:
    /// <summary>
    /// シングルトンインスタンスを取得する。
    /// </summary>
    static FontManager* GetInstance();

    /// <summary>
    /// 初期化処理。設定 JSON を読み込みフォントを登録する。
    /// </summary>
    void Initialize();

    /// <summary>
    /// 終了処理。保持している全フォントを解放する。
    /// </summary>
    void Finalize();

    /// <summary>
    /// JSON 設定ファイルからフォント定義を読み込んで登録する。
    /// </summary>
    /// <param name="_configPath">設定ファイルのパス</param>
    /// <returns>1 つ以上登録できたら true</returns>
    bool LoadFromConfig(const std::string& _configPath);

    /// <summary>
    /// フォントを登録する。同一 (path, fontSize) は再ロードせずキャッシュを共有する。
    /// TrueType の読み込みに失敗した場合はビルトイン ASCII フォントとして登録する。
    /// </summary>
    /// <param name="_name">エディタ表示等で使う識別名</param>
    /// <param name="_path">フォントファイルのパス</param>
    /// <param name="_fontSize">ベースとなるピクセルサイズ</param>
    /// <param name="_fallbackPaths">主フォントに無いグリフを補うフォールバックフォントのパス候補。
    /// 先頭から読み込みを試し、最初に成功した 1 つを採用する。</param>
    /// <returns>登録されたフォントのハンドル</returns>
    FontHandle Load(const std::string& _name, const std::string& _path, float _fontSize,
                    const std::vector<std::string>& _fallbackPaths = {});

    /// <summary>
    /// ハンドルから BitmapFont を取得する。
    /// 無効なハンドルの場合はデフォルトフォントを返す。
    /// </summary>
    /// <returns>BitmapFont ポインタ。フォントが 1 つも無ければ nullptr</returns>
    BitmapFont* GetFont(FontHandle _handle);

    /// <summary>
    /// デフォルトフォントのハンドルを取得する。
    /// </summary>
    FontHandle GetDefault() const { return defaultHandle_; }

    /// <summary>
    /// 登録済みフォントの情報。
    /// </summary>
    struct FontEntry {
        FontHandle handle = kInvalidFontHandle;
        std::string name;
        std::string path;
        float fontSize = 0.0f;
    };

    /// <summary>
    /// 登録済みフォント一覧を取得する（エディタのドロップダウン等で使用）。
    /// </summary>
    const std::vector<FontEntry>& GetFontList() const { return fontList_; }

    /// <summary>
    /// ハンドルに対応する識別名を取得する。
    /// </summary>
    const std::string& GetFontName(FontHandle _handle) const;

private:
    FontManager()                              = default;
    ~FontManager()                             = default;
    FontManager(const FontManager&)            = delete;
    FontManager& operator=(const FontManager&) = delete;

    static std::string MakeCacheKey(const std::string& _path, float _fontSize);

    std::unordered_map<FontHandle, std::unique_ptr<BitmapFont>> fonts_;
    std::unordered_map<std::string, FontHandle> cache_; // "path|size" -> handle
    std::vector<FontEntry> fontList_;

    FontHandle nextHandle_    = 0;
    FontHandle defaultHandle_ = kInvalidFontHandle;
    std::string emptyName_;
};

} // namespace OriGine

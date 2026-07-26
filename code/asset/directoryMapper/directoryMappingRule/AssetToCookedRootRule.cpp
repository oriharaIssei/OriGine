#include "AssetToCookedRootRule.h"

using namespace OriGine;

/// <summary>
/// コンストラクタ
/// </summary>
AssetToCookedRootRule::AssetToCookedRootRule() {}

/// <summary>
/// デストラクタ
/// </summary>
AssetToCookedRootRule::~AssetToCookedRootRule() {}

/// <summary>
/// 生アセットのルートディレクトリを、クック済みアセットのルートへ差し替える。
/// 例: "application/resource/texture/a.png" → "application/cookedResource/texture/a.png"
/// </summary>
/// <param name="_directory">変換元のディレクトリパス</param>
/// <returns>変換後のパス。対象外の形のパスであれば入力をそのまま返す</returns>
std::filesystem::path AssetToCookedRootRule::Apply(const std::filesystem::path& _directory) const {
    // ./ を除去
    std::filesystem::path normalized = _directory.lexically_normal();

    // このルールが変換したいパスの形は "<engine|application>/resource/..." であり、
    // それを "<engine|application>/cookedResource/..." に置き換える（生アセット→クック済みアセットへのマッピング）。
    // パスの先頭2セグメントだけを見て判定し、それ以降は変更せずそのまま連結する。

    auto it = normalized.begin();
    if (it == normalized.end()) {
        return _directory;
    }

    // 1セグメント目 (engine / application) はどちらのルートかを示す情報なのでそのまま維持する
    const std::filesystem::path first = *it; // engine / application
    ++it;
    if (it == normalized.end()) {
        return _directory;
    }

    // 2セグメント目が "resource"（生アセットのルート名）でない場合は、
    // このルールの対象外のパスなので変換せずに元のパスを返す
    if (it->string() != assetRootDirectoryName_) {
        return _directory;
    }

    // 1セグメント目 + "cookedResource" を新しいルートとして組み立てる
    std::filesystem::path result;
    result /= first;
    result /= cookedRootDirectoryName_;

    // 3セグメント目以降（"resource" より後ろのサブパス）はそのまま付け替える
    ++it;
    for (; it != normalized.end(); ++it) {
        result /= *it;
    }

    return result;
}

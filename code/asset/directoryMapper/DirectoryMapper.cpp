#include "DirectoryMapper.h"

using namespace OriGine;

/// <summary>
/// コンストラクタ
/// </summary>
DirectoryMapper::DirectoryMapper() {}

/// <summary>
/// デストラクタ
/// </summary>
DirectoryMapper::~DirectoryMapper() {}

/// <summary>
/// 初期化処理。ルールは利用側が AddRule で登録するため、ここでは何もしない
/// </summary>
void OriGine::DirectoryMapper::Initialize() {}

/// <summary>
/// 終了処理。登録済みの変換ルールをすべて破棄する
/// </summary>
void OriGine::DirectoryMapper::Finalize() {
    rules_.clear();
}

/// <summary>
/// パス変換ルールを登録する。
/// TryMap では登録順に適用されるため、呼び出す順序がそのまま変換の順序になる
/// </summary>
/// <param name="_rule">追加する変換ルール</param>
void OriGine::DirectoryMapper::AddRule(const std::shared_ptr<IDirectoryMappingRule>& _rule) {
    rules_.emplace_back(_rule);
}

/// <summary>
/// 登録済みのルールを順に適用して、ディレクトリパスを変換する
/// </summary>
/// <param name="_directory">変換元のディレクトリパス</param>
/// <returns>全ルール適用後のパス（どのルールにも該当しなければ入力と同じパス）</returns>
std::filesystem::path OriGine::DirectoryMapper::TryMap(const std::filesystem::path& _directory) const {
    std::filesystem::path mappedDirectory = _directory;
    // 登録された変換ルールを AddRule で追加された順に連鎖的に適用する（パイプライン方式）。
    // 各ルールは直前のルールが返した結果を入力として受け取るため、ルールを追加する順序が
    // 最終的なパスに影響する（例: 「アセット→クック済みルート」の変換を先に行ってから
    // 「拡張子変換」を行う、という順序に依存する使い方がされている）。
    for (const auto& rule : rules_) {
        mappedDirectory = rule->Apply(mappedDirectory);
    }
    return mappedDirectory;
}

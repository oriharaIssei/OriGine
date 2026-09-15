#pragma once

/// stl
#include <functional>
#include <string>
#include <vector>

namespace OriGine::Test {

/// <summary>
/// 1テストケースの実行結果。
/// diagnosticLines は PASS/FAIL いずれの場合にも出力する行の集まりとして1つにまとめてある。
/// 「FAIL時に期待値/実測値を出す」(ケース2/3/4)と「PASSでも観測値そのものを証拠として出す」
/// (ケース5)は、専用のフィールドを分けるより同じ入れ物で表現したほうが呼び出し側の分岐が減る。
/// </summary>
struct TestCaseResult {
    bool passed = true;
    std::vector<std::string> diagnosticLines;
};

/// <summary>
/// テストケース1つを表す実行関数の型。引数を取らず、結果を組み立てて返す。
/// </summary>
using TestCaseFunc = std::function<TestCaseResult()>;

/// <summary>
/// 名前付きテストケース1件(名前, 実行関数)。
/// 名前を関数側ではなくこちらに持たせているのは、ログ出力に使う名前と関数の対応を
/// 呼び出し側(MakeComponentTypeIdTestCases)の一箇所だけで管理し、ずれを防ぐため。
/// </summary>
struct TestCaseEntry {
    std::string name;
    TestCaseFunc func;
};

/// <summary>
/// 渡された順序どおりにテストケースを実行し、結果を集計してログ/標準出力の両方に出す。
///
/// ケース名は各ケースの実行「前」に出力し、即座にflushする(spdlogはDevelop/Releaseで
/// level::debug以上、Debugでlevel::trace以上を毎回flushする設定になっているため、
/// LOG_INFO/LOG_ERRORを呼んだ時点でファイルへの書き込みは確定する。詳細はLogger.cppを参照)。
/// これは、このスイートの一部のケースが既知のバグにより未定義動作でプロセスごと落ちうるため、
/// 「どのケースの実行中に落ちたか」をクラッシュ後のログからでも特定できるようにするための設計。
/// クラッシュした場合、この関数自体が戻ってこない(=それ以降のケースは実行されない)。
/// </summary>
/// <param name="_cases">実行するテストケース列。この配列の並び順がそのまま実行順序になる</param>
/// <param name="_suiteName">見出しに出すスイート名(複数スイート対応前は"ComponentTypeId"固定だった)</param>
/// <returns>0 = 全ケースPASS、1 = 1件以上FAIL</returns>
int RunTestCases(const std::vector<TestCaseEntry>& _cases, const std::string& _suiteName);

} // namespace OriGine::Test

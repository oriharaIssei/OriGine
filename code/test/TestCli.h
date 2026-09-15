#pragma once

/// stl
#include <string>
#include <vector>

namespace OriGine::Test {

/// <summary>
/// main.cpp の ParseCommandLine() が返す引数リストに "--test" または "--test=&lt;suite&gt;" が
/// 含まれている場合、対応するテストスイート(TestSuiteRegistry::GetAllTestSuites() を参照)を
/// 実行して結果をログ/標準出力に出したのち true を返す(呼び出し元はこれを合図に
/// アプリケーションを即座に終了させること)。
/// どちらも指定されていない場合は何もせず false を返す(既存の起動フローを一切変更しない)。
///
/// 対応するCLI引数:
///   --test           : 登録済みの全スイートを登録順に実行する(1つでもFAILがあれば全体をFAIL扱いにする)
///   --test=&lt;suite&gt; : 指定した1スイートだけを実行する。知らないスイート名の場合は
///                       何も実行せず、終了コードを1にする(黙って全件実行にフォールバックしない)
/// </summary>
/// <param name="_commandLines">main.cpp の ParseCommandLine() の戻り値</param>
/// <returns>テストを実行した場合は true</returns>
bool RunCliTestsIfRequested(const std::vector<std::string>& _commandLines);

/// <summary>
/// 直近の RunCliTestsIfRequested() 実行結果に基づく終了コード(0=全PASS, 1=1件以上FAIL)を返す。
/// RunCliTestsIfRequested() が true を返した直後にのみ意味を持つ(呼ばれていない、または
/// false だった場合は 0 のまま)。
///
/// bench の CLI には無い概念だが、テストは「合否」を返す必要があるため用意した。
/// 呼び出し元(FrameWork派生クラスのInitialize)はこの値を FrameWork::exitCode_ に積み、
/// main.cpp の WinMain がその値をプロセスの終了コードとしてそのまま返す
/// (bench.ps1 と同じ「終了コードで自動判定する」運用をテストでも成立させるための配線。
///  従来 WinMain は常に 0 を返す実装だったため、この配線を追加していないと
///  --test の結果に関わらず終了コードが常に0になり、test.ps1 側で合否判定ができない)。
/// </summary>
/// <returns>0 = 全ケースPASS、1 = 1件以上FAIL</returns>
int GetLastTestExitCode();

} // namespace OriGine::Test

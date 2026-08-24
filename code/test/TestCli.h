#pragma once

/// stl
#include <string>
#include <vector>

namespace OriGine::Test {

/// <summary>
/// main.cpp の ParseCommandLine() が返す引数リストに "--test" が含まれている場合、
/// コンポーネント型IDの寿命バグを検出する回帰テスト一式(ComponentTypeIdTests)を実行して
/// 結果をログ/標準出力に出したのち true を返す(呼び出し元はこれを合図にアプリケーションを
/// 即座に終了させること)。
/// "--test" が指定されていない場合は何もせず false を返す(既存の起動フローを一切変更しない)。
///
/// 対応するCLI引数:
///   --test  : テストモードを有効化する(これが無ければ何もしない)
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

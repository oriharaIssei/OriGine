#pragma once

/// stl
#include <vector>

/// test
#include "test/TestRunner.h"

namespace OriGine::Test {

/// <summary>
/// Phase 5(ECSハンドル + ストレージ再設計)の安全網として、EntityRepository /
/// ComponentRepository / ISystem の**公開APIを通した意味論**を固定する回帰テスト一式。
///
/// Phase 5 では EntityHandle が UUID から {index, generation} へ、3種のunordered_mapが撤廃、
/// per-entity std::vector が撤廃される予定(docs/todo.html Phase 5)。つまり「内部でUUIDの
/// unordered_mapを引いている」「EntitySlotがvectorである」といった**今の実装の形**は
/// 書き換えの対象そのものであり、これに依存したテストを書くと書き換えた瞬間に全滅して
/// 安全網として機能しない。
///
/// そのためここで固定するのは「エンティティを作って壊す」「コンポーネントを付けて取って消す」
/// 「システムに登録・解除する」を公開APIだけで行ったときに観測できる結果(値・成否・nullptrか
/// どうか)だけであり、内部表現(unordered_map の中身、vectorの本数、EntityHandleがUUIDである
/// こと自体)には一切触れない。各ケースの先頭コメントに「なぜこの振る舞いを固定するのか」を
/// 1行書いてあるので、Phase 5 後にこのスイートのどれかが落ちたら、そのコメントと照らして
/// 「実装がまだ仕様を満たしていない(事故)」か「意図して仕様を変えた(このケースごと消してよい)」
/// かを判断すること。
///
/// 既存の component-type-id / descriptor / serialize-golden スイートと同様、Scene や
/// DirectX12 デバイスは使わない。EntityRepository / ComponentRepository は単体で初期化でき、
/// Scene::CreateEntity 等はこの2つへの薄いラッパーに過ぎないため、Scene無しでも
/// 同じ公開契約を検証できる(既存3スイートも同じ理由でSceneを使っていない)。
/// </summary>
/// <returns>実行順に並んだテストケース列</returns>
std::vector<TestCaseEntry> MakeEcsSemanticsTestCases();

} // namespace OriGine::Test

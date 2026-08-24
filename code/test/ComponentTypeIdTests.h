#pragma once

/// stl
#include <vector>

/// test
#include "test/TestRunner.h"

namespace OriGine::Test {

/// <summary>
/// ComponentRepository::GetComponentArray&lt;T&gt;() の static キャッシュにまつわる
/// 「型ID の寿命バグ」を検出する回帰テスト一式を、実行すべき順序どおりに組み立てて返す。
///
/// オラクル(正解)には非テンプレート版 IComponentArray* GetComponentArray(const std::string&)
/// を使う。こちらは呼び出しの都度 ComponentRegistry から型IDを引き直すだけで、型ごとの
/// 状態を一切キャッシュしないため、常にそのインスタンスにとって正しい配列を返す。
/// 対してテンプレート版は static を経由する。かつてそこにキャッシュしていたのは
/// 「このリポジトリの vector の何番目か」という**インスタンスごとに異なる値**であり、
/// それが型ID化以前のバグの正体だった。この2つの戻り値が食い違う瞬間がバグの発現であり、
/// 各ケースは基本的に「GetComponentArray&lt;T&gt;() と GetComponentArray(nameof&lt;T&gt;()) が
/// 同じポインタを指すか」だけを見ている。
/// 現在は static が覚えているのが「型の identity(ComponentRegistry が採番した型ID)」に
/// 変わったため、全ケースが PASS する。判定式は型ID化の前後どちらでも成立するので、
/// 今後の回帰テストとしてそのまま使い続けられる。
///
/// 例外はケース4だけ: 型ID化以前の UnregisterComponentArray は vector の中間から erase して
/// 以降の添字をずらしたまま対応表を再インデックスしなかったため、テンプレート版・オラクル版の
/// 両方が同じ壊れた添字を参照し、判定式単体では検出できなかった。
/// そのためケース4だけは判定式に加えて独立の確認を行っている(現在の実装は穴を空けるだけで
/// 詰めないので、この確認も通る)。
///
/// 実行順序はケース番号どおりではない: ケース4(AfterUnregister_RemainingIndicesStayValid)は
/// 型ID化以前は componentArrays_ への範囲外アクセスを踏み、未定義動作でプロセスごと落ちうる
/// ケースだった。落ちると残りのケースが実行されないため、巻き添えを避けて必ず最後に置いている
/// (デグレで再発したときにも効くので、この配置は維持すること)。
/// ケース5(TypeIndicesAreDistinctAndWithinCap)は GetComponentArray&lt;T&gt;() を一切呼ばず
/// 型IDだけを見るため、static を汚染も参照もしない。どこに置いても他のケースに影響しないので、
/// 「壊れうるものは最後」の原則を保ったままケース4より前に済ませてしまう。
/// </summary>
/// <returns>実行順に並んだテストケース列</returns>
std::vector<TestCaseEntry> MakeComponentTypeIdTestCases();

} // namespace OriGine::Test

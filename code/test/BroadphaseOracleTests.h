#pragma once

/// stl
#include <vector>

/// test
#include "test/TestRunner.h"

namespace OriGine::Test {

/// <summary>
/// Phase 6(ブロードフェーズの再実装)の照合相手となる「正解オラクル」スイート。
///
/// ユーザーがこれから SpatialHash::GetAllPairs の std::set 実装を固定セル配列 +
/// カウンティングソートへ書き換える。書き換えの前後で「返すペア集合が変わっていないか」を
/// 機械的に確認できるよう、SpatialHash とは完全に独立した O(N^2) の総当たり実装(オラクル)を
/// ここで用意し、実際の SpatialHash::GetAllPairs の出力と集合として一致するかを照合する。
///
/// このスイートは2つの異なる契約を別々のテストケースとして検査する:
///
/// (1) セル共有の等価性(ケース1〜7)。SpatialHash.cpp自身のコメントが明言する契約は
///     「同じセルに入っているもの同士だけを候補として返す」(= 幾何学的に実際に接触しているかは
///     呼び出し側のナローフェーズが別途判定する)。したがってオラクルが判定するのも
///     「2つのAABBが1つ以上のセルを共有するか」であって、「2つのAABBが幾何学的に
///     重なっているか」ではない(この2つは別物: 例えばcellSize=100・半径1のような設定では、
///     同じセルに何十個もエンティティが入るが実際に接触しているのはごく一部、というのが
///     正常な状態であり、「幾何学的重なり」を基準に比較すると正しい実装でも大量の不一致として
///     検出されてしまう)。ここでは実装の完全一致(集合として等しいか)を見る。
///
/// (2) 幾何学的重なりの包含(ケース8)。「実際に幾何学的に重なっているペアは、SpatialHashの
///     候補集合に必ず含まれていなければならない」という、内部実装(全破棄・全再構築でも、
///     Phase 6でユーザーが行う差分更新でも、将来別のデータ構造に変わっても)によらず
///     成り立つべき最低限の安全性契約。差分更新は「更新漏れで本来重なっているペアが候補から
///     落ちる」事故が定番のため、(1)のセル共有オラクルとは別に維持する価値がある。
///     こちらはSpatialHash側の余剰候補(セルは共有しているが実際には重なっていない候補)を
///     一切問題にせず、見逃し(false negative)だけを検出する(部分集合関係の検査)。
///
/// どちらのオラクルも、セル所属判定(floor(座標 / cellSize))や重なり判定を
/// SpatialHashのPositionToCell/GetCellRangeを一切呼ばずに独立に再実装しており、
/// ペアの列挙も std::set 等の重複排除を経由せず全組み合わせを1回ずつ判定する総当たり
/// ループで行う(SpatialHash側のセル分割・ハッシュ・重複排除ロジックを一切共有しない)。
///
/// 既存の component-type-id / layout / descriptor / serialize-golden / ecs-semantics
/// スイートと同様、Scene や DirectX12 デバイスは使わない。SpatialHash::Insert /
/// GetAllPairs はEntityHandleとAABBだけで完結する公開APIであり、EntityHandleは
/// EntityRepository::CreateEntity で作れば十分なため(コンポーネントは一切不要)。
/// </summary>
/// <returns>実行順に並んだテストケース列</returns>
std::vector<TestCaseEntry> MakeBroadphaseOracleTestCases();

} // namespace OriGine::Test

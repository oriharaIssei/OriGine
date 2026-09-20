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
/// オラクルが判定する「重なり」の定義について:
/// SpatialHash.cpp 自身のコメントが明言する契約は「同じセルに入っているもの同士だけを
/// 候補として返す」(= 幾何学的に実際に接触しているかどうかは呼び出し側のナローフェーズが
/// 別途判定する)。したがってこのオラクルが判定するのも「2つのAABBが1つ以上のセルを
/// 共有するか」であって、「2つのAABBが幾何学的に重なっているか」ではない
/// (この2つは別物: 例えばcellSize=100・半径1のような設定では、同じセルに何十個も
/// エンティティが入るが実際に接触しているのはごく一部、というのが正常な状態であり、
/// 「幾何学的重なり」を基準に比較すると正しい実装でも大量の不一致として検出されてしまう)。
/// セル所属判定(floor(座標 / cellSize))はSpatialHashのPositionToCell/GetCellRangeを
/// 一切呼ばずにオラクル側で独立に再実装しており、ペアの列挙も std::set 等の重複排除を経由せず
/// 全組み合わせを1回ずつ判定する総当たりループで行う(SpatialHash側のセル分割・ハッシュ・
/// 重複排除ロジックを一切共有しない)。
///
/// 既存の component-type-id / layout / descriptor / serialize-golden / ecs-semantics
/// スイートと同様、Scene や DirectX12 デバイスは使わない。SpatialHash::Insert /
/// GetAllPairs はEntityHandleとAABBだけで完結する公開APIであり、EntityHandleは
/// EntityRepository::CreateEntity で作れば十分なため(コンポーネントは一切不要)。
/// </summary>
/// <returns>実行順に並んだテストケース列</returns>
std::vector<TestCaseEntry> MakeBroadphaseOracleTestCases();

} // namespace OriGine::Test

#pragma once

/// stl
#include <vector>

/// test
#include "test/TestRunner.h"

namespace OriGine::Test {

/// <summary>
/// 全コンポーネント葉型(54型)と、中間クラス(IComponent / ICollider / Collider&lt;Bounds&gt;各実体 /
/// MeshRenderer系 / PrimitiveMeshRendererBase)の sizeof / alignof を1行ずつ出力するスイートを
/// 組み立てて返す。
///
/// これはPhase 3(IComponent非仮想化)前の現状のレイアウトを記録するための「表を出力するだけ」の
/// スイートで、期待値を固定した合否判定は行わない(sizeofはB-5で変わる前提のため、固定すると
/// 実装のたびにこのテストを書き換える羽目になり、回帰テストとして機能しなくなる)。
/// 判定式としての役割は「コンパイルが通ること」自体が担う: 54型のうちどれか1つでも
/// リネーム・削除・移動されればこのファイルがコンパイルエラーになるため、
/// 型一覧が古くならないことを機械的に保証する。
/// </summary>
/// <returns>実行順に並んだテストケース列(いずれも診断行に表を積むだけで、通常は必ずPASSする)</returns>
std::vector<TestCaseEntry> MakeLayoutTestCases();

} // namespace OriGine::Test

#pragma once

/// stl
#include <string>
#include <vector>

/// test
#include "test/TestRunner.h"

namespace OriGine::Test {

/// <summary>
/// 1つのテストスイート。名前と、ケース列を組み立てる関数を1組にしたもの。
/// makeCases を関数ポインタにしているのは、スイートを増やすたびに新しいラムダの型が
/// 増えて GetAllTestSuites() の戻り値の型が崩れるのを避けるため(全スイートで同じ
/// シグネチャ std::vector&lt;TestCaseEntry&gt;() に揃えれば、生の関数ポインタで足りる)。
/// </summary>
struct TestSuite {
    std::string name;
    std::vector<TestCaseEntry> (*makeCases)();
};

/// <summary>
/// 登録済みの全スイートを、登録順(= --test 実行時の実行順)で返す。
/// 新しいスイートを追加する場合は TestSuiteRegistry.cpp の一覧に足すだけでよい。
/// </summary>
const std::vector<TestSuite>& GetAllTestSuites();

} // namespace OriGine::Test

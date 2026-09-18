#pragma once

/// stl
#include <vector>

/// test
#include "test/TestRunner.h"

namespace OriGine::Test {

/// <summary>
/// Phase 3C(型ディスクリプタのコード生成)の確認用スイート。
///
/// このスイートは「動いたかどうか」だけを判定する。表の合計バイト数・型の数・
/// フィールド行の数といった生の数字は診断行に積んで出力するだけで、Claude 側では
/// 解釈もコメントも行わない(CLAUDE.md: 数字はユーザーが読んで判断する)。
///
/// 回帰として固定しているのは構造的な不変条件だけ:
/// - FieldDesc/TypeDesc が設計どおりの32/24バイトであること(static_assert と二重の確認)
/// - 対象10型それぞれについて、型IDから引いた TypeDesc が存在し、型名・sizeof(T)・
///   フィールド数がヘッダの実体と一致すること(ヘッダを編集して反映対象が変わっても
///   生成をし忘れるとここで壊れる)
/// - 各型が指す FieldDesc の範囲が共有配列の境界内に収まり、各フィールドの
///   offset_ + size_ が所属型の sizeof を超えないこと(生成ツールのオフセット計算が
///   壊れていないかの機械的な下限チェック)
/// </summary>
/// <returns>実行順に並んだテストケース列</returns>
std::vector<TestCaseEntry> MakeDescriptorTestCases();

} // namespace OriGine::Test

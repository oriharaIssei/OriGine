#pragma once
// ============================================================================
// 自動生成ファイル。手で編集しないこと。
// 生成元: project/engine/tools/ReflectionCodeGen
// 対象: project/engine/tools/ReflectionCodeGen/targets.txt に列挙された型
// 再生成: premake のプリビルドで ReflectionCodeGen.exe が実行される。
// 内容が変わらない限りファイルは書き換わらない(タイムスタンプも更新されない)。
// ============================================================================

namespace OriGine {

/// <summary>
/// targets.txt に列挙された型の TypeDesc/FieldDesc を、ComponentReflection.h の
/// 64要素の型IDテーブルへ登録する。FrameWork.cpp の RegisterUsingComponents() から
/// 明示的に呼ぶこと(Q16: 静的初期化子による自己登録は静的ライブラリで
/// リンカに捨てられるため使わない)。
/// </summary>
void RegisterGeneratedComponentDescriptors();

} // namespace OriGine

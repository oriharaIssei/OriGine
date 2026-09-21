#pragma once
// ============================================================================
// 自動生成ファイル。手で編集しないこと。
// 生成元: project/engine/tools/ReflectionCodeGen
// 対象: project/engine/tools/ReflectionCodeGen/targets.txt に列挙された型
// 再生成: premake のプリビルドで ReflectionCodeGen.exe が実行される。
// 内容が変わらない限りファイルは書き換わらない(タイムスタンプも更新されない)。
// ============================================================================

/// DLL境界(Phase 4 4D)。RegisterUsingComponents() はアプリ側(EXE)からこの関数を
/// 直接呼ぶため、OriGine.dll 側では実体をエクスポートする必要がある。
#include "OriGineApi.h"

namespace OriGine {

/// <summary>
/// targets.txt に列挙された型の TypeDesc/FieldDesc を、ComponentReflection.h の
/// 64要素の型IDテーブル(コンポーネント)、および入れ子構造体専用テーブルへ登録する。
/// FrameWork.cpp の RegisterUsingComponents() から明示的に呼ぶこと(Q16: 静的初期化子に
/// よる自己登録は静的ライブラリでリンカに捨てられるため使わない)。
/// </summary>
ORIGINE_API void RegisterGeneratedComponentDescriptors();

} // namespace OriGine

#pragma once

/// <summary>
/// OriGine を DLL として構成したときの、モジュール境界を越えるシンボルへの注釈.
///
/// なぜ3分岐あるのか(2つではなく):
///  - OriGine プロジェクト自身をビルドするときは ORIGINE_BUILD_DLL を定義し、
///    エクスポート(dllexport)にする。
///  - OriGine.dll を読む側(ECS_TestGame / ECS_TestEditor)は ORIGINE_USE_DLL を定義し、
///    インポート(dllimport)にする。
///  - どちらも定義しない第3の分岐(空)を必ず残す。engine/premake.lua は standalone モード
///    (engine 単体を StaticLib としてビルドする確認用。App 側 workspace を介さず
///    `premake5.exe vs2026` を直接叩く)を持っており、そこでは DLL 境界が存在しない。
///    2分岐(BUILD/USE)しか無いと standalone ビルドが dllimport のまま解決できず壊れる。
/// </summary>
#if defined(ORIGINE_BUILD_DLL)
#    define ORIGINE_API __declspec(dllexport)
#elif defined(ORIGINE_USE_DLL)
#    define ORIGINE_API __declspec(dllimport)
#else
#    define ORIGINE_API
#endif

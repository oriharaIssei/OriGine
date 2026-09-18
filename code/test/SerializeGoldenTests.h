#pragma once

/// stl
#include <vector>

/// test
#include "test/TestRunner.h"

namespace OriGine::Test {

/// <summary>
/// Phase 3 A-5(ゴールデンJSONと往復テスト)のスイート。
///
/// 目的は「今の手書き to_json/from_json が出すJSONを正解として固定し、3D(シリアライズを
/// ディスクリプタ経由に書き換える段階)の前後で同じ結果になることを機械で確かめられるようにする」こと
/// (docs/plans/phase-03.md 4章 A-5、6章の罠7〜9番)。シリアライズ本体の書き換えはこのスイートの
/// 範囲外で、現状の挙動を固定するだけ(3Dより前にコミットが必須)。
///
/// ゴールデンJSONの実体は project/engine/resource/test/serialize-golden/ にコミットしてある
/// (generated/ はgitignore対象のため使えない。6章の罠9番「セーブデータの実物がない」への対応)。
///
/// 確認する3点(タスク指示の1/2/3に対応):
/// 1. 読む→書く→一致: golden-current.json を ComponentArray&lt;T&gt;::LoadComponents で読み込み、
///    そのまま SaveComponents で書き出した結果が元のJSONとキー・値ともに一致すること
///    (RoundTrip_CurrentTypes)。対象は3Dでディスクリプタ経由に変える型(Transformほか、
///    3Cで注釈を付けた public のみの10型のうち9型。SmoothingEffectParamは既存バグ
///    (to_json/from_jsonがOriGine名前空間の外に定義されておりADL経由でリンクできない)により
///    対象外。SerializeGoldenTests.cpp末尾のコメント参照)と、手書きのまま残る型の代表4つ
///    (Rigidbody/SphereCollider/CollisionPushBackInfo/DistortionEffectParam)の計13型。
/// 2. 旧形式: golden-legacy-input.json(キーが欠けたJSON)を読み込み、from_json側の既定値で
///    補われた結果が golden-legacy-expected.json と一致すること(RoundTrip_LegacyDefaults)。
///    DistortionEffectParam の綴りミスキー "textuerPath" への互換フォールバックも含む。
/// 3. 型名キーと"Handle": 上記1・2のいずれも ComponentArray&lt;T&gt; 経由で保存しており、
///    トップレベルのキーが nameof&lt;T&gt;() と一致すること、各要素が "Handle" キーを保ち、
///    その値が保存時のものと変わらないことをあわせて確認する(各ケース内の CheckRoundTrip/
///    CheckLegacyRoundTrip が明示的に検証する)。
/// </summary>
/// <returns>実行順に並んだテストケース列</returns>
std::vector<TestCaseEntry> MakeSerializeGoldenTestCases();

} // namespace OriGine::Test

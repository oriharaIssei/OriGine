#include "test/SerializeGoldenTests.h"

/// stl
#include <cmath>
#include <format>
#include <fstream>
#include <string>
#include <vector>

/// externals
#include <nlohmann/json.hpp>
#include <uuid/uuid.h>

/// ECS
#include "component/ComponentArray.h"

#include "component/transform/Transform.h"
#include "component/transform/Transform2d.h"
#include "component/transform/CameraTransform.h"
#include "component/material/light/DirectionalLight.h"
#include "component/material/light/PointLight.h"
#include "component/material/light/SpotLight.h"
#include "component/effect/post/OutlineComponent.h"
#include "component/text/TextComponent.h"
#include "component/text/TextStreamComponent.h"

#include "component/physics/Rigidbody.h"
#include "component/collision/collider/SphereCollider.h"
#include "component/collision/collider/base/CollisionCategoryManager.h"
#include "component/collision/CollisionPushBackInfo.h"
#include "component/effect/post/DistortionEffectParam.h"

/// util
#include <util/nameof.h>

/// logger
#include "logger/Logger.h"

using namespace OriGine;

namespace OriGine::Test {

namespace {

// ゴールデンJSONの実体は generated/ (gitignore対象)ではなく engine/resource 配下にコミットしてある
// (docs/plans/phase-03.md 6章の罠9番: 「セーブデータの実物がない」への対応。3Dより前に固定必須)。
// パスは test.ps1 が exe を project/ を作業ディレクトリにして起動する前提(kEngineResourceDirectoryと同じ規則)。
constexpr const char* kGoldenCurrentPath         = "./engine/resource/test/serialize-golden/golden-current.json";
constexpr const char* kGoldenLegacyInputPath     = "./engine/resource/test/serialize-golden/golden-legacy-input.json";
constexpr const char* kGoldenLegacyExpectedPath  = "./engine/resource/test/serialize-golden/golden-legacy-expected.json";

// 全ケースで使い回す固定のEntityHandle。ComponentArray<T>はT毎に独立したインスタンスを
// このテスト内で都度new/破棄するため、同じuuidを使い回しても型をまたいだ衝突は起きない。
EntityHandle MakeTestEntityHandle() {
    auto uuid = uuids::uuid::from_string("aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa");
    return EntityHandle(uuid.value());
}

/// <summary>
/// JSONファイルを読み込む。見つからない/パース失敗時は false を返し、診断行に理由を積む
/// (計測器と同じ原則: 読めなかったことを黙って握りつぶさない)。
/// </summary>
bool LoadJsonFile(const std::string& _path, nlohmann::json& _out, std::vector<std::string>& _diags) {
    std::ifstream ifs(_path);
    if (!ifs) {
        _diags.push_back(std::format("[FAIL] golden JSONを開けなかった: {}", _path));
        return false;
    }
    try {
        ifs >> _out;
    } catch (const nlohmann::json::parse_error& e) {
        _diags.push_back(std::format("[FAIL] golden JSONのパースに失敗: {} ({})", _path, e.what()));
        return false;
    }
    return true;
}

// 浮動小数の丸め(float<->double往復、テキストのparse/dump)を吸収するための許容誤差。
// 絶対誤差・相対誤差のどちらか緩い方を満たせばよい(小さい値と大きい値の両方をカバーするため)。
constexpr double kFloatAbsEpsilon = 1e-4;
constexpr double kFloatRelEpsilon = 1e-5;

bool NearlyEqual(double _a, double _b) {
    double diff  = std::fabs(_a - _b);
    double scale = (std::max)(std::fabs(_a), std::fabs(_b));
    return diff <= (std::max)(kFloatAbsEpsilon, kFloatRelEpsilon * scale);
}

/// <summary>
/// 2つのJSON値を再帰的に比較する。数値は上記の許容誤差付き、それ以外(文字列/真偽値/null/
/// オブジェクトのキー集合/配列長)は完全一致を要求する。オブジェクトはキーの並び順を見ない
/// (nlohmann::jsonの内部表現に依存しないよう、contains()/at()で引き当てる)。
/// 不一致箇所は _diffs にパス付きで積み、呼び出し側が「どのキーがどう違うか」を出力できるようにする。
/// </summary>
bool DeepEqual(const nlohmann::json& _golden, const nlohmann::json& _actual, const std::string& _path, std::vector<std::string>& _diffs) {
    // 数値同士は先に見る(number_integer/number_unsigned/number_floatの型差を吸収するため、
    // is_number()同士なら下のtype()一致チェックより先に許容誤差比較へ回す)。
    if (_golden.is_number() && _actual.is_number()) {
        double gv = _golden.get<double>();
        double av = _actual.get<double>();
        if (!NearlyEqual(gv, av)) {
            _diffs.push_back(std::format("[FAIL] {}: golden={} actual={} (float tolerance exceeded)", _path, gv, av));
            return false;
        }
        return true;
    }

    if (_golden.type() != _actual.type()) {
        _diffs.push_back(std::format("[FAIL] {}: type mismatch golden={} actual={}", _path, _golden.type_name(), _actual.type_name()));
        return false;
    }

    if (_golden.is_object()) {
        bool ok = true;
        // 書くが読み戻さない/読み戻すが書かないキーを先に検出する(6章の罠7/8番、A-4節3の
        // GpuParticleEmitter::isEmitEdgeのような非対称パターンを見逃さないための総当たりチェック)。
        for (auto it = _golden.begin(); it != _golden.end(); ++it) {
            if (!_actual.contains(it.key())) {
                _diffs.push_back(std::format("[FAIL] {}.{}: goldenにはあるが往復後の出力に無い(書くが読み戻さない、またはその逆の疑い)", _path, it.key()));
                ok = false;
            }
        }
        for (auto it = _actual.begin(); it != _actual.end(); ++it) {
            if (!_golden.contains(it.key())) {
                _diffs.push_back(std::format("[FAIL] {}.{}: 往復後の出力にはあるがgoldenに無い", _path, it.key()));
                ok = false;
            }
        }
        for (auto it = _golden.begin(); it != _golden.end(); ++it) {
            if (_actual.contains(it.key())) {
                ok &= DeepEqual(it.value(), _actual.at(it.key()), _path + "." + it.key(), _diffs);
            }
        }
        return ok;
    }

    if (_golden.is_array()) {
        if (_golden.size() != _actual.size()) {
            _diffs.push_back(std::format("[FAIL] {}: array size golden={} actual={}", _path, _golden.size(), _actual.size()));
            return false;
        }
        bool ok = true;
        for (size_t i = 0; i < _golden.size(); ++i) {
            ok &= DeepEqual(_golden[i], _actual[i], std::format("{}[{}]", _path, i), _diffs);
        }
        return ok;
    }

    // string / bool / null
    if (_golden != _actual) {
        _diffs.push_back(std::format("[FAIL] {}: golden={} actual={}", _path, _golden.dump(), _actual.dump()));
        return false;
    }
    return true;
}

/// <summary>
/// 1型分の往復を確認する。_inputRoot から読み込み、ComponentArray&lt;T&gt;::SaveComponents で
/// 書き出した結果を _expectedRoot と比較する。「現行データの往復」("読む→書く→一致"の1番)は
/// _inputRoot と _expectedRoot に同じgolden(golden-current.json)を渡せばよく、
/// 「旧形式のデフォルト補完」(2番)は _inputRoot に golden-legacy-input.json、
/// _expectedRoot に golden-legacy-expected.json を渡す。
///
/// あわせて次の2点も確認する(3番): nameof&lt;T&gt;() が _typeLabel(=golden側のトップレベルキー)と
/// 一致すること、SaveComponents の出力がその1キーだけを持つこと("Handle"の保持自体は
/// _expectedRoot 側にも "Handle" を含めてあるので DeepEqual が値ごと確認する)。
/// </summary>
template <IsComponent T>
void CheckRoundTrip(const std::string& _typeLabel, const nlohmann::json& _inputRoot, const nlohmann::json& _expectedRoot, TestCaseResult& _result) {
    std::vector<std::string> diags;
    bool ok = true;

    const std::string actualTypeName = nameof<T>();
    if (actualTypeName != _typeLabel) {
        ok = false;
        diags.push_back(std::format(
            "[FAIL] 型名キー不一致: golden側キー='{}' nameof<T>()='{}' (3Dでキーの生成規則が変わった疑い)",
            _typeLabel, actualTypeName));
    }

    if (!_inputRoot.contains(_typeLabel) || !_expectedRoot.contains(_typeLabel)) {
        ok = false;
        diags.push_back(std::format(
            "[FAIL] golden JSONにトップレベルキー'{}'が無い(input={}, expected={})",
            _typeLabel, _inputRoot.contains(_typeLabel), _expectedRoot.contains(_typeLabel)));
        _result.passed &= ok;
        _result.diagnosticLines.push_back(std::format("[FAIL] {}", _typeLabel));
        for (const std::string& d : diags) {
            _result.diagnosticLines.push_back(d);
        }
        return;
    }

    const nlohmann::json& inputArray    = _inputRoot.at(_typeLabel);
    const nlohmann::json& expectedArray = _expectedRoot.at(_typeLabel);

    EntityHandle entity = MakeTestEntityHandle();

    ComponentArray<T> arr;
    arr.Initialize();
    arr.RegisterEntity(entity);
    arr.LoadComponents(entity, inputArray, HandleAssignMode::UseSaved);

    nlohmann::json outRoot;
    bool saved = arr.SaveComponents(entity, outRoot);

    if (!saved) {
        ok = false;
        diags.push_back(std::format("[FAIL] {}: SaveComponentsがfalseを返した(RegisterEntityが効いていない?)", _typeLabel));
    } else if (outRoot.size() != 1 || !outRoot.contains(_typeLabel)) {
        ok = false;
        diags.push_back(std::format(
            "[FAIL] {}: SaveComponentsの出力の形が想定外(トップレベルキーは'{}' 1つだけのはず): {}",
            _typeLabel, _typeLabel, outRoot.dump()));
    } else {
        ok &= DeepEqual(expectedArray, outRoot.at(_typeLabel), _typeLabel, diags);
    }

    _result.passed &= ok;
    _result.diagnosticLines.push_back(std::format("[{}] {}", ok ? "ok  " : "FAIL", _typeLabel));
    for (const std::string& d : diags) {
        _result.diagnosticLines.push_back(d);
    }
}

/// <summary>
/// ケース1: golden-current.json を使い、「読む→書く→一致」(1番)と「型名キー・Handle保持」(3番)を、
/// 3Dでディスクリプタ経由に変える型(3Cで注釈を付けたpublicのみの10型のうち9型。SmoothingEffectParamは
/// 既存バグにより対象外。本ファイル末尾のコメント、および golden-current.json の "_readme" を参照)と、
/// 手書きのまま残る型の代表4つ(Rigidbody/SphereCollider/CollisionPushBackInfo/DistortionEffectParam)の
/// 計13型で確認する。
/// </summary>
TestCaseResult RoundTrip_CurrentTypes() {
    TestCaseResult result;
    result.passed = true;

    nlohmann::json golden;
    if (!LoadJsonFile(kGoldenCurrentPath, golden, result.diagnosticLines)) {
        result.passed = false;
        return result;
    }

    // SphereColliderのcollisionCategoryは"GoldenTestCategory"という、既定の"Default"とは別の
    // カテゴリ名を使う(未登録カテゴリ名を使うとCollisionCategoryManagerが常にdefaultCategory_
    // (名前"Default")を返してしまい、往復比較が「たまたま両方Defaultで一致」という弱いテストに
    // なってしまうため)。テスト側で明示的に登録してから読み込む。
    CollisionCategoryManager::GetInstance()->GetOrRegisterCategory("GoldenTestCategory");

    // 3Dでディスクリプタ経由に変える型(3Cで注釈を付けたpublicのみの10型のうち9型。
    // SmoothingEffectParamは対象外。理由はこのファイル末尾のコメントを参照)
    CheckRoundTrip<Transform>("Transform", golden, golden, result);
    CheckRoundTrip<Transform2d>("Transform2d", golden, golden, result);
    CheckRoundTrip<CameraTransform>("CameraTransform", golden, golden, result);
    CheckRoundTrip<DirectionalLight>("DirectionalLight", golden, golden, result);
    CheckRoundTrip<PointLight>("PointLight", golden, golden, result);
    CheckRoundTrip<SpotLight>("SpotLight", golden, golden, result);
    CheckRoundTrip<OutlineComponent>("OutlineComponent", golden, golden, result);
    CheckRoundTrip<TextComponent>("TextComponent", golden, golden, result);
    CheckRoundTrip<TextStreamComponent>("TextStreamComponent", golden, golden, result);

    // 手書きのまま残る型の代表4つ
    CheckRoundTrip<Rigidbody>("Rigidbody", golden, golden, result);
    CheckRoundTrip<SphereCollider>("SphereCollider", golden, golden, result);
    CheckRoundTrip<CollisionPushBackInfo>("CollisionPushBackInfo", golden, golden, result);
    CheckRoundTrip<DistortionEffectParam>("DistortionEffectParam", golden, golden, result);

    return result;
}

/// <summary>
/// ケース2: golden-legacy-input.json(キーが欠けた旧形式)を読み込み、from_json側の既定値/
/// contains()補完で golden-legacy-expected.json と同じ結果になることを確認する(2番)。
/// DistortionEffectParam は綴りミスキー "textuerPath" への互換フォールバックも兼ねる。
/// </summary>
TestCaseResult RoundTrip_LegacyDefaults() {
    TestCaseResult result;
    result.passed = true;

    nlohmann::json legacyInput;
    nlohmann::json legacyExpected;
    if (!LoadJsonFile(kGoldenLegacyInputPath, legacyInput, result.diagnosticLines)) {
        result.passed = false;
        return result;
    }
    if (!LoadJsonFile(kGoldenLegacyExpectedPath, legacyExpected, result.diagnosticLines)) {
        result.passed = false;
        return result;
    }

    CheckRoundTrip<Rigidbody>("Rigidbody", legacyInput, legacyExpected, result);
    CheckRoundTrip<DistortionEffectParam>("DistortionEffectParam", legacyInput, legacyExpected, result);

    return result;
}

} // namespace

std::vector<TestCaseEntry> MakeSerializeGoldenTestCases() {
    return {
        {"RoundTrip_CurrentTypes", RoundTrip_CurrentTypes},
        {"RoundTrip_LegacyDefaults", RoundTrip_LegacyDefaults},
    };
}

} // namespace OriGine::Test

// SmoothingEffectParam を対象から外した理由(golden-current.json の "_readme" にも同じ内容を記載):
// project/engine/code/ECS/component/effect/post/SmoothingEffectParam.cpp の to_json/from_json は
// `namespace OriGine { ... }` の外(グローバル名前空間)に定義されている。クラス内の friend 宣言は
// OriGine 名前空間内で OriGine::to_json/OriGine::from_json を宣言するが、.cpp 側の定義はそれとは
// 別のシンボル(::to_json/::from_json)になるため、ADL 経由で SmoothingEffectParam をシリアライズ
// しようとするコード(nlohmann::json への代入や get<T>())はリンクできない(LNK2019)。
// このスイートを実装するまで SmoothingEffectParam は一度もシリアライズ経路を通っていなかった
// (登録済み9型に含まれず、他のテストもsizeofしか見ていない)ため、これまで気づかれていなかった。
// シリアライズ本体の書き換えは3Dの範囲であり、このタスク(A-5)では修正しない(報告のみ)。

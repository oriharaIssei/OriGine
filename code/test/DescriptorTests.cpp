#include "test/DescriptorTests.h"

/// stl
#include <cstddef>
#include <format>
#include <string>
#include <vector>

/// ECS
#include "component/ComponentReflection.h"
#include "component/ComponentRegistry.h"
#include "component/FieldStrategy.h"

#include "component/transform/Transform.h"
#include "component/transform/Transform2d.h"
#include "component/transform/CameraTransform.h"
#include "component/material/light/DirectionalLight.h"
#include "component/material/light/PointLight.h"
#include "component/material/light/SpotLight.h"
#include "component/effect/post/OutlineComponent.h"
#include "component/effect/post/SmoothingEffectParam.h"
#include "component/text/TextComponent.h"
#include "component/text/TextStreamComponent.h"

/// math
#include "math/Matrix3x3.h"
#include "math/Matrix4x4.h"

/// util
#include <util/nameof.h>

using namespace OriGine;

namespace OriGine::Test {

namespace {

/// <summary>
/// 対象10型のうち1つについて、型IDからディスクリプタを引き直せること・
/// 型名/サイズ/フィールド数がヘッダの実体と一致することを確認する。
/// _expectedFieldCount は本ファイルの著者(このコード生成タスクを実装した際)が
/// ヘッダを1つずつ数えた値。ヘッダ側でフィールドを増減して生成をし忘れると
/// ここが食い違って FAIL する(回帰テストとしての役割)。
/// </summary>
template <IsComponent T>
void CheckType(const char* _name, uint32_t _expectedFieldCount, TestCaseResult& _result) {
    uint32_t typeId          = GetComponentTypeId<T>();
    const TypeDesc* desc     = GetTypeDescriptor(typeId);

    if (!desc) {
        _result.passed = false;
        _result.diagnosticLines.push_back(std::format("[FAIL] {}: TypeDesc が登録されていない(typeId={})", _name, typeId));
        return;
    }

    bool nameOk  = (std::string(desc->typeName_) == nameof<T>());
    bool sizeOk  = (desc->typeSize_ == static_cast<uint32_t>(sizeof(T)));
    bool countOk = (desc->fieldCount_ == _expectedFieldCount);
    bool boundsOk = (static_cast<uint64_t>(desc->fieldStart_) + desc->fieldCount_ <= GetFieldTableCount());

    bool fieldsFitOk = true;
    if (boundsOk) {
        const FieldDesc* fields = GetFieldTable();
        for (uint32_t i = 0; i < desc->fieldCount_; ++i) {
            const FieldDesc& f = fields[desc->fieldStart_ + i];
            if (static_cast<uint64_t>(f.offset_) + f.size_ > desc->typeSize_) {
                fieldsFitOk = false;
            }
        }
    }

    bool ok = nameOk && sizeOk && countOk && boundsOk && fieldsFitOk;
    _result.passed &= ok;
    _result.diagnosticLines.push_back(std::format(
        "[{}] {:<20} typeId={:<3} typeName={:<20} sizeof={:<6} (desc={:<6}) fields={:<3} (expected {:<3}) fieldRange=[{},{}) boundsOk={} fieldsFitOk={}",
        ok ? "ok  " : "FAIL", _name, typeId, desc->typeName_, sizeof(T), desc->typeSize_,
        desc->fieldCount_, _expectedFieldCount, desc->fieldStart_, desc->fieldStart_ + desc->fieldCount_,
        boundsOk, fieldsFitOk));
}

/// <summary>
/// ケース1: FieldDesc/TypeDesc のバイト数と、表全体の合計バイト数・型数・フィールド数を
/// 診断行に出力する。判定式は静的レイアウト(32/24バイト)だけで、合計バイト数そのものは
/// 出力するだけで合否には使わない(ユーザーが生の数字を読んで判断するため。CLAUDE.md)。
/// </summary>
TestCaseResult DescriptorTableShape() {
    TestCaseResult result;
    result.passed = true;

    result.passed &= (sizeof(FieldDesc) == 32);
    result.passed &= (sizeof(TypeDesc) == 24);

    uint32_t fieldRowCount = GetFieldTableCount();

    // 型の数は、対象10型それぞれの型IDを引いて GetTypeDescriptor が非nullを返す数を数える
    // (64要素の表を先頭から舐めるより、意図が明確なので対象10型のIDを直接使う)。
    uint32_t typeRowCount = 0;
    if (GetTypeDescriptor(GetComponentTypeId<Transform>())) ++typeRowCount;
    if (GetTypeDescriptor(GetComponentTypeId<Transform2d>())) ++typeRowCount;
    if (GetTypeDescriptor(GetComponentTypeId<CameraTransform>())) ++typeRowCount;
    if (GetTypeDescriptor(GetComponentTypeId<DirectionalLight>())) ++typeRowCount;
    if (GetTypeDescriptor(GetComponentTypeId<PointLight>())) ++typeRowCount;
    if (GetTypeDescriptor(GetComponentTypeId<SpotLight>())) ++typeRowCount;
    if (GetTypeDescriptor(GetComponentTypeId<OutlineComponent>())) ++typeRowCount;
    if (GetTypeDescriptor(GetComponentTypeId<SmoothingEffectParam>())) ++typeRowCount;
    if (GetTypeDescriptor(GetComponentTypeId<TextComponent>())) ++typeRowCount;
    if (GetTypeDescriptor(GetComponentTypeId<TextStreamComponent>())) ++typeRowCount;

    uint64_t totalBytes = static_cast<uint64_t>(fieldRowCount) * sizeof(FieldDesc)
                           + static_cast<uint64_t>(typeRowCount) * sizeof(TypeDesc);

    result.diagnosticLines.push_back(std::format("sizeof(FieldDesc)={}", sizeof(FieldDesc)));
    result.diagnosticLines.push_back(std::format("sizeof(TypeDesc)={}", sizeof(TypeDesc)));
    result.diagnosticLines.push_back(std::format("field row count={}", fieldRowCount));
    result.diagnosticLines.push_back(std::format("type row count={}", typeRowCount));
    result.diagnosticLines.push_back(std::format("total bytes = field*{} + type*{} = {}",
        sizeof(FieldDesc), sizeof(TypeDesc), totalBytes));

    return result;
}

/// <summary>
/// ケース2: 対象10型それぞれについて、型ID経由でディスクリプタが正しく引けることを確認する。
/// </summary>
TestCaseResult DescriptorPerTypeConsistency() {
    TestCaseResult result;
    result.passed = true;

    CheckType<Transform>("Transform", 5, result);
    CheckType<Transform2d>("Transform2d", 5, result);
    CheckType<CameraTransform>("CameraTransform", 9, result);
    CheckType<DirectionalLight>("DirectionalLight", 5, result);
    CheckType<PointLight>("PointLight", 8, result);
    CheckType<SpotLight>("SpotLight", 11, result);
    CheckType<OutlineComponent>("OutlineComponent", 3, result);
    CheckType<SmoothingEffectParam>("SmoothingEffectParam", 2, result);
    CheckType<TextComponent>("TextComponent", 14, result);
    CheckType<TextStreamComponent>("TextStreamComponent", 9, result);

    return result;
}

/// <summary>
/// enum の往復確認専用のテスト用型。対象10型に実在する TextAlign(下地1バイト)に加え、
/// 「enum class の既定の下地(int、符号付き4バイト)」を確かめるためだけのローカル型を1つ足す
/// (下地が符号付きでも make_unsigned で同じ幅の符号無し整数として読む、という規則が
/// 負の列挙子でも壊れないことを見るのが目的)。
/// </summary>
enum class TestEnumDefaultInt {
    Negative = -1,
    Zero     = 0,
    Large    = 123456,
};

/// <summary>
/// ケース3: 型ディスクリプタ(FieldDesc/TypeDesc)ではなく、その1段下にある
/// ストラテジー表(component/FieldStrategy.h の FieldTypeList/kFieldTagOf/GetFieldStrategy)を
/// 直接たたく。「フィールド名→タグ→FieldStrategy<T>」という間接を経ずに、FieldTypeList の
/// 全エントリについて「値を入れる→Saveで書き出す→別の値をLoadで書き戻す→元の値と一致する」
/// ことを確認する(ストラテジー化タスクの検証手順3番)。
///
/// _obj/_field は表経由シリアライズの本番コードと同じ形(FieldDesc + 生ポインタ)で組み立てる。
/// FieldDesc::name_/jsonKey_/size_/typeTag_ 以外は使わないため、offset_ は常に0でよい
/// (このテストは1フィールド分のメモリを直接指すため、コンポーネント内オフセットは不要)。
/// </summary>
template <typename T>
void CheckStrategyRoundTrip(const char* _label, const T& _valueToSave, T _valueBeforeLoad, TestCaseResult& _result) {
    const uint8_t tag              = kFieldTagOf<T>;
    const IFieldStrategy* strategy = GetFieldStrategy(tag);
    if (!strategy) {
        _result.passed = false;
        _result.diagnosticLines.push_back(std::format("[FAIL] {}: GetFieldStrategy(tag={})がnullptr", _label, tag));
        return;
    }

    FieldDesc field{};
    field.name_    = _label;
    field.jsonKey_ = "value";
    field.size_    = static_cast<uint32_t>(sizeof(T));
    field.typeTag_ = tag;

    T saved = _valueToSave;
    nlohmann::json json;
    strategy->Save(json, field, &saved);

    T loaded = std::move(_valueBeforeLoad);
    strategy->Load(json.at("value"), field, &loaded);

    bool ok = (loaded == _valueToSave);
    _result.passed &= ok;
    _result.diagnosticLines.push_back(std::format("[{}] {} (tag={})", ok ? "ok  " : "FAIL", _label, tag));
}

/// <summary>
/// 保存非対応(Matrix3x3/Matrix4x4)の型は、例外を投げず・クラッシュせず、
/// 「JSONに何も書かれない」という形で非対応が現れることだけを確認する
/// (LogUnsupportedSave/LogUnsupportedLoad はログに残るだけで戻り値を持たないため、
/// このテストからは「クラッシュしないこと」と「書き込まれないこと」しか見えない。
/// それがこの安全網の仕様どおりの振る舞い)。
/// </summary>
template <typename T>
void CheckStrategyUnsupported(const char* _label, const T& _value, TestCaseResult& _result) {
    const uint8_t tag              = kFieldTagOf<T>;
    const IFieldStrategy* strategy = GetFieldStrategy(tag);
    if (!strategy) {
        _result.passed = false;
        _result.diagnosticLines.push_back(std::format("[FAIL] {}: GetFieldStrategy(tag={})がnullptr", _label, tag));
        return;
    }

    FieldDesc field{};
    field.name_    = _label;
    field.jsonKey_ = "value";
    field.size_    = static_cast<uint32_t>(sizeof(T));
    field.typeTag_ = tag;

    T copy = _value;
    nlohmann::json json;
    strategy->Save(json, field, &copy); // ここで例外/クラッシュしないことも確認事項のうち

    bool unsupported = !json.contains("value");
    _result.passed &= unsupported;
    _result.diagnosticLines.push_back(std::format("[{}] {} は保存非対応として扱われる(tag={})", unsupported ? "ok  " : "FAIL", _label, tag));
}

/// <summary>
/// ケース3: FieldTypeList の全エントリ(具体型17 + EnumAs&lt;U&gt;4)について Save/Load の往復を
/// 確認する(Matrix3x3/Matrix4x4 は往復ではなく「非対応」であることを確認する。EnumAs&lt;U&gt;は
/// 実際の列挙型経由でのみ呼べるため、TextAlign/TestEnumDefaultInt で間接的に確認する)。
/// </summary>
TestCaseResult FieldStrategyRoundTrip() {
    TestCaseResult result;
    result.passed = true;

    CheckStrategyRoundTrip<bool>("bool", true, false, result);
    CheckStrategyRoundTrip<int32_t>("int32_t", -5, 0, result);
    CheckStrategyRoundTrip<uint32_t>("uint32_t", 5u, 0u, result);
    CheckStrategyRoundTrip<uint64_t>("uint64_t", 123456789012345ull, 0ull, result);
    CheckStrategyRoundTrip<float>("float", 1.5f, 0.0f, result);
    CheckStrategyRoundTrip<Vec2f>("Vec2f", Vec2f(1.0f, 2.0f), Vec2f(0.0f, 0.0f), result);
    CheckStrategyRoundTrip<Vec3f>("Vec3f", Vec3f(1.0f, 2.0f, 3.0f), Vec3f(0.0f, 0.0f, 0.0f), result);
    CheckStrategyRoundTrip<Vec4f>("Vec4f", Vec4f(1.0f, 2.0f, 3.0f, 4.0f), Vec4f(0.0f, 0.0f, 0.0f, 0.0f), result);
    CheckStrategyRoundTrip<Quaternion>("Quaternion", Quaternion(0.0f, 0.0f, 0.0f, 1.0f), Quaternion(1.0f, 0.0f, 0.0f, 0.0f), result);
    CheckStrategyRoundTrip<std::string>("std::string", std::string("hello"), std::string(""), result);
    CheckStrategyRoundTrip<std::vector<float>>("vector<float>", {1.0f, 2.0f, 3.0f}, {}, result);
    CheckStrategyRoundTrip<std::vector<int32_t>>("vector<int32_t>", {1, 2, 3}, {}, result);
    CheckStrategyRoundTrip<std::vector<Vec2f>>("vector<Vec2f>", {Vec2f(1.0f, 2.0f)}, {}, result);
    CheckStrategyRoundTrip<std::vector<Vec3f>>("vector<Vec3f>", {Vec3f(1.0f, 2.0f, 3.0f)}, {}, result);
    CheckStrategyRoundTrip<std::vector<Vec4f>>("vector<Vec4f>", {Vec4f(1.0f, 2.0f, 3.0f, 4.0f)}, {}, result);

    CheckStrategyUnsupported<Matrix3x3>("Matrix3x3", MakeMatrix3x3::Identity(), result);
    CheckStrategyUnsupported<Matrix4x4>("Matrix4x4", Matrix4x4{}, result);

    // enum: EnumAs<U> 経由(kFieldTagOfが列挙型からEnumAs<make_unsigned_t<underlying_type_t<E>>>
    // へ委譲する経路)。下地1バイト(実在するTextAlign)と4バイト(符号付きintが既定のenum class)
    // の両方を確かめる。
    CheckStrategyRoundTrip<TextAlign>("TextAlign(u8)", TextAlign::Right, TextAlign::Left, result);
    CheckStrategyRoundTrip<TestEnumDefaultInt>("TestEnumDefaultInt(i32)", TestEnumDefaultInt::Negative, TestEnumDefaultInt::Zero, result);
    CheckStrategyRoundTrip<TestEnumDefaultInt>("TestEnumDefaultInt(i32,large)", TestEnumDefaultInt::Large, TestEnumDefaultInt::Zero, result);

    return result;
}

} // namespace

std::vector<TestCaseEntry> MakeDescriptorTestCases() {
    return {
        {"DescriptorTableShape", DescriptorTableShape},
        {"DescriptorPerTypeConsistency", DescriptorPerTypeConsistency},
        {"FieldStrategyRoundTrip", FieldStrategyRoundTrip},
    };
}

} // namespace OriGine::Test

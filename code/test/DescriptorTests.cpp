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

/// <summary>
/// 入れ子構造体(ORIGINE_STRUCT())専用テーブルは ComponentTypeId を持たないため、
/// GetTypeDescriptor(typeId) では引けない。名前で舐めて探す(登録数は数個なので線形探索で十分)。
/// </summary>
const TypeDesc* FindNestedTypeDescByName(const char* _name) {
    const TypeDesc* table = GetNestedTypeTable();
    uint32_t count         = GetNestedTypeTableCount();
    for (uint32_t i = 0; i < count; ++i) {
        if (std::string(table[i].typeName_) == _name) {
            return &table[i];
        }
    }
    return nullptr;
}

/// <summary>_desc の中から名前が一致する FieldDesc を探す。無ければ nullptr。</summary>
const FieldDesc* FindFieldByName(const TypeDesc& _desc, const char* _name) {
    const FieldDesc* fields = GetFieldTable();
    for (uint32_t i = 0; i < _desc.fieldCount_; ++i) {
        const FieldDesc& f = fields[_desc.fieldStart_ + i];
        if (std::string(f.name_) == _name) {
            return &f;
        }
    }
    return nullptr;
}

/// <summary>
/// ケース4: 入れ子構造体(OutlineParamData/BoxFilterSize)専用の TypeDesc が正しく登録されて
/// いること・フィールド数と型タグが期待どおりであることを確認する(型ディスクリプタに
/// 入れ子構造体を追加したタスクの検証手順3番)。
/// </summary>
TestCaseResult NestedStructDescriptor() {
    TestCaseResult result;
    result.passed = true;

    const TypeDesc* outlineParam = FindNestedTypeDescByName("OutlineParamData");
    if (!outlineParam) {
        result.passed = false;
        result.diagnosticLines.push_back("[FAIL] OutlineParamData: 入れ子構造体専用テーブルに見つからない");
    } else {
        bool sizeOk  = outlineParam->typeSize_ == static_cast<uint32_t>(sizeof(OutlineParamData));
        bool countOk = outlineParam->fieldCount_ == 2; // outlineWidth, outlineColor
        result.passed &= sizeOk && countOk;
        result.diagnosticLines.push_back(std::format(
            "[{}] OutlineParamData fields={} (expected 2) sizeof={} (desc={})",
            (sizeOk && countOk) ? "ok  " : "FAIL", outlineParam->fieldCount_, sizeof(OutlineParamData), outlineParam->typeSize_));

        const FieldDesc* width = FindFieldByName(*outlineParam, "outlineWidth");
        const FieldDesc* color = FindFieldByName(*outlineParam, "outlineColor");
        bool tagOk = width && color && width->typeTag_ == kFieldTagOf<float> && color->typeTag_ == kFieldTagOf<Vec4f>;
        result.passed &= tagOk;
        result.diagnosticLines.push_back(std::format("[{}] OutlineParamData のフィールド型タグが期待どおり", tagOk ? "ok  " : "FAIL"));
    }

    const TypeDesc* boxFilterSize = FindNestedTypeDescByName("BoxFilterSize");
    if (!boxFilterSize) {
        result.passed = false;
        result.diagnosticLines.push_back("[FAIL] BoxFilterSize: 入れ子構造体専用テーブルに見つからない");
    } else {
        bool sizeOk  = boxFilterSize->typeSize_ == static_cast<uint32_t>(sizeof(BoxFilterSize));
        bool countOk = boxFilterSize->fieldCount_ == 1; // size
        result.passed &= sizeOk && countOk;
        result.diagnosticLines.push_back(std::format(
            "[{}] BoxFilterSize fields={} (expected 1) sizeof={} (desc={})",
            (sizeOk && countOk) ? "ok  " : "FAIL", boxFilterSize->fieldCount_, sizeof(BoxFilterSize), boxFilterSize->typeSize_));

        const FieldDesc* size = FindFieldByName(*boxFilterSize, "size");
        bool tagOk = size && size->typeTag_ == kFieldTagOf<Vec2f>;
        result.passed &= tagOk;
        result.diagnosticLines.push_back(std::format("[{}] BoxFilterSize::size の型タグが期待どおり", tagOk ? "ok  " : "FAIL"));
    }

    return result;
}

/// <summary>
/// ケース5: 入れ子フィールドのオフセット計算の正解照合。OutlineComponent/SmoothingEffectParam を
/// スタックに1つ作り(Initialize()は呼ばない。GPUバッファを作らないためGPUデバイス不要)、
/// ディスクリプタ(FieldDesc::offset_ を辿って入れ子TypeDescへ入り、さらにそのFieldDesc::offset_
/// を辿る)経由で計算したアドレスが、実際のC++メンバのアドレス(&comp.paramData.openData_.
/// outlineWidth 等)と一致することを確認する。これが「FieldUnwrap&lt;IConstantBuffer&lt;X&gt;&gt;の
/// オフセット計算が正しいこと」の正解オラクルになる。
/// </summary>
TestCaseResult NestedFieldOffsetOracle() {
    TestCaseResult result;
    result.passed = true;

    // --- OutlineComponent::paramData -> OutlineParamData::outlineWidth/outlineColor ---
    {
        OutlineComponent outline; // Initialize()は呼ばない(paramData.CreateBufferがGPUを要求するため)

        const TypeDesc* outlineDesc = GetTypeDescriptor(GetComponentTypeId<OutlineComponent>());
        const FieldDesc* paramDataField = outlineDesc ? FindFieldByName(*outlineDesc, "paramData") : nullptr;
        if (!paramDataField) {
            result.passed = false;
            result.diagnosticLines.push_back("[FAIL] OutlineComponent::paramData の FieldDesc が見つからない");
        } else {
            const std::byte* base    = reinterpret_cast<const std::byte*>(&outline);
            const void* paramDataPtr = base + paramDataField->offset_; // FieldUnwrap込みで openData_ の先頭を指すはず

            bool baseOk = (paramDataPtr == static_cast<const void*>(&outline.paramData.openData_));
            result.passed &= baseOk;
            result.diagnosticLines.push_back(std::format(
                "[{}] OutlineComponent::paramData の offset_ が &paramData.openData_ と一致", baseOk ? "ok  " : "FAIL"));

            const TypeDesc* nestedTable = GetNestedTypeTable();
            uint32_t nestedCount         = GetNestedTypeTableCount();
            if (paramDataField->nestedTypeIndex_ >= nestedCount) {
                result.passed = false;
                result.diagnosticLines.push_back("[FAIL] OutlineComponent::paramData の nestedTypeIndex_ が範囲外");
            } else {
                const TypeDesc& nestedDesc  = nestedTable[paramDataField->nestedTypeIndex_];
                const std::byte* nestedBase = reinterpret_cast<const std::byte*>(paramDataPtr);

                const FieldDesc* width = FindFieldByName(nestedDesc, "outlineWidth");
                const FieldDesc* color = FindFieldByName(nestedDesc, "outlineColor");
                bool widthOk = width && (nestedBase + width->offset_ == reinterpret_cast<const std::byte*>(&outline.paramData.openData_.outlineWidth));
                bool colorOk = color && (nestedBase + color->offset_ == reinterpret_cast<const std::byte*>(&outline.paramData.openData_.outlineColor));
                result.passed &= widthOk && colorOk;
                result.diagnosticLines.push_back(std::format("[{}] OutlineParamData::outlineWidth のアドレスが一致", widthOk ? "ok  " : "FAIL"));
                result.diagnosticLines.push_back(std::format("[{}] OutlineParamData::outlineColor のアドレスが一致", colorOk ? "ok  " : "FAIL"));
            }
        }
    }

    // --- SmoothingEffectParam::boxFilterSize_ -> BoxFilterSize::size ---
    {
        SmoothingEffectParam smoothing; // Initialize()は呼ばない(boxFilterSize_.CreateBufferがGPUを要求するため)

        const TypeDesc* smoothingDesc = GetTypeDescriptor(GetComponentTypeId<SmoothingEffectParam>());
        const FieldDesc* boxFilterSizeField = smoothingDesc ? FindFieldByName(*smoothingDesc, "boxFilterSize_") : nullptr;
        if (!boxFilterSizeField) {
            result.passed = false;
            result.diagnosticLines.push_back("[FAIL] SmoothingEffectParam::boxFilterSize_ の FieldDesc が見つからない");
        } else {
            const std::byte* base   = reinterpret_cast<const std::byte*>(&smoothing);
            const void* boxSizePtr = base + boxFilterSizeField->offset_;

            bool baseOk = (boxSizePtr == static_cast<const void*>(&smoothing.boxFilterSize_.openData_));
            result.passed &= baseOk;
            result.diagnosticLines.push_back(std::format(
                "[{}] SmoothingEffectParam::boxFilterSize_ の offset_ が &boxFilterSize_.openData_ と一致", baseOk ? "ok  " : "FAIL"));

            const TypeDesc* nestedTable = GetNestedTypeTable();
            uint32_t nestedCount         = GetNestedTypeTableCount();
            if (boxFilterSizeField->nestedTypeIndex_ >= nestedCount) {
                result.passed = false;
                result.diagnosticLines.push_back("[FAIL] SmoothingEffectParam::boxFilterSize_ の nestedTypeIndex_ が範囲外");
            } else {
                const TypeDesc& nestedDesc  = nestedTable[boxFilterSizeField->nestedTypeIndex_];
                const std::byte* nestedBase = reinterpret_cast<const std::byte*>(boxSizePtr);

                const FieldDesc* size = FindFieldByName(nestedDesc, "size");
                bool sizeOk = size && (nestedBase + size->offset_ == reinterpret_cast<const std::byte*>(&smoothing.boxFilterSize_.openData_.size));
                result.passed &= sizeOk;
                result.diagnosticLines.push_back(std::format("[{}] BoxFilterSize::size のアドレスが一致", sizeOk ? "ok  " : "FAIL"));
            }
        }
    }

    return result;
}

/// <summary>
/// ケース6: 入れ子構造体用ストラテジー(FieldStrategy&lt;NestedStructTag&gt;)のSave/Load往復確認。
/// OutlineComponent::paramData を対象に、Saveすると入れ子オブジェクト({"outlineWidth":...,
/// "outlineColor":[...]})になること、Loadで元の値がそのまま復元できることを見る。
/// </summary>
TestCaseResult NestedStrategyRoundTrip() {
    TestCaseResult result;
    result.passed = true;

    const TypeDesc* outlineDesc = GetTypeDescriptor(GetComponentTypeId<OutlineComponent>());
    const FieldDesc* paramDataField = outlineDesc ? FindFieldByName(*outlineDesc, "paramData") : nullptr;
    if (!paramDataField) {
        result.passed = false;
        result.diagnosticLines.push_back("[FAIL] OutlineComponent::paramData の FieldDesc が見つからない");
        return result;
    }

    const IFieldStrategy* strategy = GetFieldStrategy(paramDataField->typeTag_);
    if (!strategy) {
        result.passed = false;
        result.diagnosticLines.push_back(std::format("[FAIL] GetFieldStrategy(tag={}) が nullptr", paramDataField->typeTag_));
        return result;
    }

    OutlineComponent src;
    src.paramData.openData_.outlineWidth = 1.25f;
    src.paramData.openData_.outlineColor = Vec4f(0.1f, 0.2f, 0.3f, 0.4f);

    nlohmann::json j;
    strategy->Save(j, *paramDataField, &src.paramData.openData_);

    bool isNestedObject = j.contains(paramDataField->jsonKey_) && j.at(paramDataField->jsonKey_).is_object();
    result.passed &= isNestedObject;
    result.diagnosticLines.push_back(std::format("[{}] Save結果がキー'{}'の入れ子オブジェクトになっている",
        isNestedObject ? "ok  " : "FAIL", paramDataField->jsonKey_));

    if (!isNestedObject) {
        return result;
    }

    OutlineComponent dst; // 既定値(src とは異なる)から始める
    strategy->Load(j.at(paramDataField->jsonKey_), *paramDataField, &dst.paramData.openData_);

    bool widthOk = (dst.paramData.openData_.outlineWidth == src.paramData.openData_.outlineWidth);
    bool colorOk = (dst.paramData.openData_.outlineColor == src.paramData.openData_.outlineColor);
    result.passed &= widthOk && colorOk;
    result.diagnosticLines.push_back(std::format("[{}] Load後にoutlineWidthが往復して一致", widthOk ? "ok  " : "FAIL"));
    result.diagnosticLines.push_back(std::format("[{}] Load後にoutlineColorが往復して一致", colorOk ? "ok  " : "FAIL"));

    return result;
}

/// <summary>_desc から _fieldName を探し、no_save/read_only のビットが期待どおりかを見る。</summary>
void CheckFieldFlags(const TypeDesc* _desc, const char* _typeLabel, const char* _fieldName,
    bool _expectNoSave, bool _expectReadOnly, TestCaseResult& _result) {
    if (!_desc) {
        _result.passed = false;
        _result.diagnosticLines.push_back(std::format("[FAIL] {}: TypeDesc が見つからない", _typeLabel));
        return;
    }
    const FieldDesc* f = FindFieldByName(*_desc, _fieldName);
    if (!f) {
        _result.passed = false;
        _result.diagnosticLines.push_back(std::format("[FAIL] {}::{} の FieldDesc が見つからない", _typeLabel, _fieldName));
        return;
    }
    bool noSave   = (f->flags_ & kFieldFlagNoSave) != 0;
    bool readOnly = (f->flags_ & kFieldFlagReadOnly) != 0;
    bool ok       = (noSave == _expectNoSave) && (readOnly == _expectReadOnly);
    _result.passed &= ok;
    _result.diagnosticLines.push_back(std::format(
        "[{}] {}::{} no_save={} (expected {}) read_only={} (expected {})",
        ok ? "ok  " : "FAIL", _typeLabel, _fieldName, noSave, _expectNoSave, readOnly, _expectReadOnly));
}

/// <summary>
/// ケース7: no_save と read_only の分離(B)が分類どおりに反映されていることを確認する。
/// 期待値の根拠(どのシステムが書き込むか)は各フィールドの ORIGINE_FIELD コメントと、
/// タスクの報告に書いた分類表を参照。ここでは反映結果だけを機械的に見る。
///
/// Matrix3x3/Matrix4x4 と Opaque(生ポインタ)は FieldStrategy 側で常に読み取り専用になるため、
/// read_only フラグは立てていない(付けても実害は無いが、既に別経路で保証されているものに
/// 冗長な注釈を重ねない、という判断)。worldMat/viewMat/projectionMat/parent がこれに当たる。
/// </summary>
TestCaseResult FieldFlagClassification() {
    TestCaseResult result;
    result.passed = true;

    // no_saveのみ(read_onlyではない) = 保存しないが編集は可能。入れ子の中身を編集させるための核心。
    CheckFieldFlags(GetTypeDescriptor(GetComponentTypeId<OutlineComponent>()), "OutlineComponent", "paramData", true, false, result);
    CheckFieldFlags(GetTypeDescriptor(GetComponentTypeId<SmoothingEffectParam>()), "SmoothingEffectParam", "boxFilterSize_", true, false, result);

    // no_save かつ read_only = システムが実行時に書き換えるランタイム状態。
    const TypeDesc* textDesc = GetTypeDescriptor(GetComponentTypeId<TextComponent>());
    CheckFieldFlags(textDesc, "TextComponent", "visibleCharCount", true, true, result);
    CheckFieldFlags(textDesc, "TextComponent", "dirty", true, true, result);

    const TypeDesc* streamDesc = GetTypeDescriptor(GetComponentTypeId<TextStreamComponent>());
    CheckFieldFlags(streamDesc, "TextStreamComponent", "revealed", true, true, result);
    CheckFieldFlags(streamDesc, "TextStreamComponent", "elapsedDelay", true, true, result);
    CheckFieldFlags(streamDesc, "TextStreamComponent", "finished", true, true, result);
    CheckFieldFlags(streamDesc, "TextStreamComponent", "lastApplied", true, true, result);
    CheckFieldFlags(streamDesc, "TextStreamComponent", "textHash", true, true, result);

    // no_saveのみ(read_onlyではない) = ユーザーが設定する値だが保存対象外(理由は各フィールドの
    // コメント参照)。canUseMainCamera はどのシステムからも書き込まれていない(grep で確認済み)。
    // viewMat/projectionMat/worldMat/parent は Matrix/Opaque ストラテジー側で読み取り専用になる
    // ため、read_only フラグ自体は立てない。
    const TypeDesc* cameraDesc = GetTypeDescriptor(GetComponentTypeId<CameraTransform>());
    CheckFieldFlags(cameraDesc, "CameraTransform", "canUseMainCamera", true, false, result);
    CheckFieldFlags(cameraDesc, "CameraTransform", "viewMat", true, false, result);
    CheckFieldFlags(cameraDesc, "CameraTransform", "projectionMat", true, false, result);

    const TypeDesc* transformDesc = GetTypeDescriptor(GetComponentTypeId<Transform>());
    CheckFieldFlags(transformDesc, "Transform", "worldMat", true, false, result);
    CheckFieldFlags(transformDesc, "Transform", "parent", true, false, result);

    const TypeDesc* transform2dDesc = GetTypeDescriptor(GetComponentTypeId<Transform2d>());
    CheckFieldFlags(transform2dDesc, "Transform2d", "worldMat", true, false, result);
    CheckFieldFlags(transform2dDesc, "Transform2d", "parent", true, false, result);

    return result;
}

/// <summary>
/// ケース8: A(DLLの中でのコンポーネント登録)の検証。Transform2d/OutlineComponent が
/// ComponentRegistry へ実際に登録され(ComponentArray を作れる状態)、名前引きの経路
/// (ComponentRegistry::FindTypeId)とテンプレートキャッシュの経路(GetComponentTypeId&lt;T&gt;())が
/// 同じ型IDに収束することを見る(docs/plans/phase-04.md: 関数ローカル static がモジュールごとに
/// 別実体になる問題への確認)。
///
/// SmoothingEffectParam は対象外: RegisterEngineEditorComponents() が意図的に登録していない
/// (to_json/from_jsonの名前空間バグにより、DLLの内側であってもLNK2019になるため。報告参照)。
///
/// 注記(スコープの限界): このテストスイート自体が OriGine.dll 側(code/test/)にリンクされて
/// いるため、ここでの確認は「DLL 内の複数翻訳単位間で一致する」ことまでしか見られない。
/// EXE 側(FrameWork.cpp)はこの2型について意図的に RegisterComponent&lt;T&gt;()/
/// GetComponentTypeId&lt;T&gt;() を一切呼ばない(それ自体が LNK2019 を避ける設計)ため、
/// EXE 側との厳密な意味でのモジュール跨ぎ確認にはならない。実際に EXE 側で
/// RegisterComponent&lt;T&gt;() を呼び、DLL 側の生成コードで GetComponentTypeId&lt;T&gt;() を
/// 呼ぶという組み合わせは、Transform で DescriptorPerTypeConsistency(CheckType&lt;Transform&gt;)
/// が既にカバーしている。
/// </summary>
void CheckEngineEditorComponentRegistered(const char* _typeName, uint32_t _typeIdFromTemplate, bool _hasArray, TestCaseResult& _result) {
    uint32_t typeIdFromName = ComponentRegistry::GetInstance()->FindTypeId(_typeName);
    const TypeDesc* desc     = GetTypeDescriptor(_typeIdFromTemplate);

    bool idOk   = (typeIdFromName != kInvalidComponentTypeId) && (typeIdFromName == _typeIdFromTemplate);
    bool descOk = desc && (desc->typeId_ == _typeIdFromTemplate);
    bool ok     = idOk && descOk && _hasArray;

    _result.passed &= ok;
    _result.diagnosticLines.push_back(std::format(
        "[{}] {}: RegisterComponent済み={} typeId(template)={} typeId(name)={} typeId(desc)={}",
        ok ? "ok  " : "FAIL", _typeName, _hasArray, _typeIdFromTemplate, typeIdFromName,
        desc ? desc->typeId_ : kInvalidComponentTypeId));
}

TestCaseResult EngineEditorComponentRegistration() {
    TestCaseResult result;
    result.passed = true;

    CheckEngineEditorComponentRegistered("Transform2d", GetComponentTypeId<Transform2d>(),
        ComponentRegistry::GetInstance()->HasComponentArray<Transform2d>(), result);
    CheckEngineEditorComponentRegistered("OutlineComponent", GetComponentTypeId<OutlineComponent>(),
        ComponentRegistry::GetInstance()->HasComponentArray<OutlineComponent>(), result);

    return result;
}

} // namespace

std::vector<TestCaseEntry> MakeDescriptorTestCases() {
    return {
        {"DescriptorTableShape", DescriptorTableShape},
        {"DescriptorPerTypeConsistency", DescriptorPerTypeConsistency},
        {"FieldStrategyRoundTrip", FieldStrategyRoundTrip},
        {"NestedStructDescriptor", NestedStructDescriptor},
        {"NestedFieldOffsetOracle", NestedFieldOffsetOracle},
        {"NestedStrategyRoundTrip", NestedStrategyRoundTrip},
        {"FieldFlagClassification", FieldFlagClassification},
        {"EngineEditorComponentRegistration", EngineEditorComponentRegistration},
    };
}

} // namespace OriGine::Test

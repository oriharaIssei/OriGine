#include "test/DescriptorTests.h"

/// stl
#include <cstddef>
#include <format>
#include <string>
#include <vector>

/// ECS
#include "component/ComponentReflection.h"
#include "component/ComponentRegistry.h"

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

} // namespace

std::vector<TestCaseEntry> MakeDescriptorTestCases() {
    return {
        {"DescriptorTableShape", DescriptorTableShape},
        {"DescriptorPerTypeConsistency", DescriptorPerTypeConsistency},
    };
}

} // namespace OriGine::Test

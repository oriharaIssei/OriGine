#pragma once

#include <string>
#include <vector>

#include "Model.h"

namespace ReflectionCodeGen {

/// <summary>
/// FieldInfo::typeSignature を FieldTypeTag に分類する(D3 の閉じた列挙)。
/// enum の判定は全ファイルを読み終えたあとの enum registry を使うため、
/// パース段階ではなく全ファイルの解析が終わった後にこの関数で行う。
/// </summary>
/// <param name="_typeSignature">"Vec3f" 等、空白を除いたトークン連結文字列</param>
/// <param name="_enums">全ファイルから集めた enum class の一覧</param>
/// <param name="_outEnumIndex">typeTag==Enum のとき、_enums 内での添字を書き込む</param>
FieldTypeTag ClassifyFieldType(const std::string& _typeSignature, const std::vector<EnumInfo>& _enums, size_t& _outEnumIndex);

} // namespace ReflectionCodeGen

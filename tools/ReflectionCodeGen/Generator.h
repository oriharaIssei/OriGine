#pragma once

#include <string>
#include <vector>

#include "Model.h"

namespace ReflectionCodeGen {

struct GeneratedFiles {
    std::string headerText;
    std::string cppText;
};

/// <summary>
/// 分類済みの型一覧と enum 一覧から、生成 .h / .cpp のテキストを組み立てる。
/// D2: 全型のフィールドを1本の静的配列(kFields)に並べ、型ごとに(開始位置, 個数)で指す。
///
/// FieldDesc.offset_ / .size_ と TypeDesc.typeSize_ は、このツールが数値を計算するのではなく
/// 生成した .cpp の中で offsetof(...) / sizeof(...) をそのまま使う。理由:
/// このツールはエンジンのヘッダに依存しない(=実際のクラスレイアウトを知らない)。
/// さらに std::string 等の一部 STL 型は MSVC の Debug 構成(イテレータデバッグ)で
/// Develop/Release とサイズが変わるため、仮にツール側で独自にオフセットを
/// 計算できたとしても、1回の生成で3構成すべてに通用する値にはならない。
/// 実際にコンパイルする側の offsetof/sizeof に任せるのが唯一正しい方法。
/// </summary>
GeneratedFiles Generate(const std::vector<ClassifiedType>& _types, const std::vector<EnumInfo>& _enums);

} // namespace ReflectionCodeGen

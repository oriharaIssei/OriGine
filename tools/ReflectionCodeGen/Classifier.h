#pragma once

#include <string>
#include <vector>

#include "Model.h"

namespace ReflectionCodeGen {

/// <summary>
/// フィールドの型テキストが、解析済み enum 一覧のどれかの名前と一致するかを調べる。
/// 旧バージョンはここで具体型(bool/Vec3f/...)まで含めて FieldTypeTag に分類していたが、
/// 具体型の判定は生成した .cpp の中で `kFieldTagOf<decltype(OriGine::Type::field)>` として
/// コンパイラに任せるようにしたため不要になった(D3の switch/if連鎖の廃止。
/// component/FieldStrategy.h 参照)。
///
/// ここに残っているのは enum の名前一致だけ: 同じ「Enum」というタグの中でも、
/// 列挙型ごとに下地の整数サイズ(EnumDesc::underlyingSize_)が違うため、どの EnumDesc を
/// 指すかという添字だけはコンパイラの decltype だけでは引けず(型名の文字列的な同定が要る)、
/// このツールが解決して数値として渡す必要がある。
/// </summary>
/// <param name="_typeSignature">"TextAlign" 等、空白を除いたトークン連結文字列</param>
/// <param name="_enums">全ファイルから集めた enum class の一覧</param>
/// <returns>一致した enum の _enums 内での添字。一致しなければ -1</returns>
int FindEnumIndex(const std::string& _typeSignature, const std::vector<EnumInfo>& _enums);

} // namespace ReflectionCodeGen

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

/// <summary>
/// フィールドの型トークン列(空白で連結する前の個々のトークン)に、ORIGINE_STRUCT() が付いた
/// 入れ子構造体の名前がトークンとして含まれるかを調べる。FindEnumIndexが「型シグネチャ全体が
/// enum名と完全一致するか」を見るのに対し、こちらは「トークン列のどこかに構造体名と完全一致する
/// トークンがあるか」を見る(例: "IConstantBuffer&lt;OutlineParamData&gt;" というシグネチャの中の
/// "OutlineParamData" トークンを見つける)。連結後の文字列に対する部分文字列一致にしないのは、
/// 例えば構造体名が "Size" だった場合に無関係な型名 "BoxFilterSize" の一部として誤検出しない
/// ようにするため。
/// </summary>
/// <param name="_typeTokens">フィールドの型を構成する個々のトークン文字列</param>
/// <param name="_structNames">ORIGINE_STRUCT() が付いた型の名前一覧。発見順(=Generatorが組む
/// 入れ子構造体専用のTypeDesc表の並び順)であること</param>
/// <param name="_fileName">曖昧一致(複数の構造体名が同時に該当)を検出したときのエラー表示用</param>
/// <param name="_sourceLine">同上</param>
/// <returns>一致した構造体の _structNames 内での添字。一致しなければ -1</returns>
/// <exception cref="ParseError">2つ以上の異なる構造体名が同時にトークンとして現れた場合(曖昧なため対応しない)</exception>
int FindNestedStructIndex(const std::vector<std::string>& _typeTokens, const std::vector<std::string>& _structNames,
    const std::string& _fileName, int _sourceLine);

} // namespace ReflectionCodeGen

#pragma once

#include <string>

#include "Model.h"
#include "Tokenizer.h"

namespace ReflectionCodeGen {

/// <summary>
/// ヘッダ1本分のソースを解析し、ORIGINE_COMPONENT() が付いた型(通常1個)と
/// enum class の一覧を返す。
///
/// 対応する構文(docs/plans/phase-03.md 6章15番の一覧に対応):
///   namespace ブロック、#pragma / #include 等の前処理行(中身は読まず1行まるごと無視)、
///   struct/class の既定アクセス差、public/private/protected ラベル、friend 宣言、
///   using/typedef、enum class(下地の型つき)、ブレース初期化子を持つフィールド、
///   コンストラクタの初期化子リスト、入れ子の struct/class(反映せずまるごと読み飛ばす)。
/// テンプレート基底は対象10型のどれも使わないため未対応(使われていたらエラーで止まる)。
///
/// 読めない構文に当たったら ParseError を投げる(黙って無視しない)。
/// </summary>
/// <param name="_src">ヘッダの生テキスト</param>
/// <param name="_fileName">エラーメッセージに使うファイル名</param>
/// <param name="_headerIncludePath">生成 .cpp が書く #include のパス</param>
FileParseResult ParseFile(const std::string& _src, const std::string& _fileName, const std::string& _headerIncludePath);

} // namespace ReflectionCodeGen

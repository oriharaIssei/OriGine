#pragma once

// ============================================================================
// ReflectionCodeGen: 最小限の C++ トークナイザ。
//
// なぜ本物のプリプロセッサ/パーサを使わないのか:
//   このツールはエンジンのヘッダに依存させない(単体でビルドできること)という制約があり、
//   コンパイラ本体や libclang のような重量級の依存を追加できない。対象は
//   「ORIGINE_COMPONENT() が付いた10型」というごく限られた構文の集合なので、
//   コメント除去 + 字句分割 + 素朴な深さ管理という最小限の実装で足りる。
//   読めない構文(想定外のトークン列)に当たったらエラーを投げて止まる方針を徹底し、
//   「なんとなく動く」を狙わない。
// ============================================================================

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace ReflectionCodeGen {

enum class TokenKind {
    Identifier, // 識別子・キーワード(区別しない。呼び出し側が文字列で判定する)
    Number,
    StringLiteral,
    CharLiteral,
    Punct, // 1文字または2文字の記号(:: , < > * & = ; { } ( ) : ~)
    End,
};

struct Token {
    TokenKind kind = TokenKind::End;
    std::string text;
    int line = 0;
};

/// <summary>
/// パース中に読めない構文へ当たったときに投げる例外。
/// main() で捕まえてエラーメッセージを出し、非0で終了する。
/// </summary>
struct ParseError : std::runtime_error {
    ParseError(const std::string& _file, int _line, const std::string& _message)
        : std::runtime_error(_file + ":" + std::to_string(_line) + ": " + _message) {}
};

/// <summary>
/// ソース全体からコメント(// と /* */)を空白に置き換える。
/// 文字列/文字リテラルの中身は保持したまま行う(対象10ヘッダのクラス本体には
/// 文字列リテラルは出てこないが、#include 行等ファイル全体には出てくるため)。
/// 改行はそのまま残すので、以降の行番号計算がずれない。
/// </summary>
std::string StripComments(const std::string& _src);

/// <summary>
/// コメント除去済みのソースを字句分割する。
/// </summary>
/// <param name="_fileName">エラーメッセージに使うファイル名</param>
std::vector<Token> Tokenize(const std::string& _src, const std::string& _fileName);

} // namespace ReflectionCodeGen

#include "Tokenizer.h"

#include <cctype>

namespace ReflectionCodeGen {

std::string StripComments(const std::string& _src) {
    std::string out;
    out.reserve(_src.size());

    size_t i = 0;
    const size_t n = _src.size();
    while (i < n) {
        char c = _src[i];
        if (c == '/' && i + 1 < n && _src[i + 1] == '/') {
            // 行コメント: 改行まで空白に置き換える(改行自体は残す)
            while (i < n && _src[i] != '\n') {
                out.push_back(' ');
                ++i;
            }
            continue;
        }
        if (c == '/' && i + 1 < n && _src[i + 1] == '*') {
            out.push_back(' ');
            out.push_back(' ');
            i += 2;
            while (i < n && !(_src[i] == '*' && i + 1 < n && _src[i + 1] == '/')) {
                out.push_back(_src[i] == '\n' ? '\n' : ' ');
                ++i;
            }
            if (i < n) {
                out.push_back(' ');
                out.push_back(' ');
                i += 2;
            }
            continue;
        }
        if (c == '"') {
            // 文字列リテラル: 中身はそのまま出力し、字句分割側で1トークンとして拾う
            out.push_back(c);
            ++i;
            while (i < n && _src[i] != '"') {
                if (_src[i] == '\\' && i + 1 < n) {
                    out.push_back(_src[i]);
                    out.push_back(_src[i + 1]);
                    i += 2;
                    continue;
                }
                out.push_back(_src[i]);
                ++i;
            }
            if (i < n) {
                out.push_back(_src[i]);
                ++i;
            }
            continue;
        }
        if (c == '\'') {
            out.push_back(c);
            ++i;
            while (i < n && _src[i] != '\'') {
                if (_src[i] == '\\' && i + 1 < n) {
                    out.push_back(_src[i]);
                    out.push_back(_src[i + 1]);
                    i += 2;
                    continue;
                }
                out.push_back(_src[i]);
                ++i;
            }
            if (i < n) {
                out.push_back(_src[i]);
                ++i;
            }
            continue;
        }
        out.push_back(c);
        ++i;
    }
    return out;
}

namespace {
bool IsIdentStart(char _c) { return std::isalpha(static_cast<unsigned char>(_c)) || _c == '_'; }
bool IsIdentCont(char _c) { return std::isalnum(static_cast<unsigned char>(_c)) || _c == '_'; }
} // namespace

std::vector<Token> Tokenize(const std::string& _src, const std::string& _fileName) {
    std::vector<Token> tokens;
    int line = 1;
    size_t i = 0;
    const size_t n = _src.size();

    while (i < n) {
        char c = _src[i];

        if (c == '\n') {
            ++line;
            ++i;
            continue;
        }
        if (std::isspace(static_cast<unsigned char>(c))) {
            ++i;
            continue;
        }
        // プリプロセッサ行は行末までをまるごと1トークン(Punct "#...")として渡す。
        // 個々のディレクティブ文法(#include の引用符付きパス等)を理解する必要が無く、
        // 呼び出し側(Parser)は「アノテーション付き型の中に #if/#ifdef/#ifndef が
        // 出てきたらエラー」の判定にテキストの中身だけを見ればよい(Q14)。
        if (c == '#') {
            size_t start = i;
            while (i < n && _src[i] != '\n') {
                ++i;
            }
            Token t;
            t.kind = TokenKind::Punct;
            t.text = _src.substr(start, i - start);
            t.line = line;
            tokens.push_back(t);
            continue;
        }
        if (IsIdentStart(c)) {
            size_t start = i;
            while (i < n && IsIdentCont(_src[i])) {
                ++i;
            }
            Token t;
            t.kind = TokenKind::Identifier;
            t.text = _src.substr(start, i - start);
            t.line = line;
            tokens.push_back(t);
            continue;
        }
        if (std::isdigit(static_cast<unsigned char>(c))) {
            size_t start = i;
            while (i < n && (std::isalnum(static_cast<unsigned char>(_src[i])) || _src[i] == '.' || _src[i] == '\'')) {
                ++i;
            }
            Token t;
            t.kind = TokenKind::Number;
            t.text = _src.substr(start, i - start);
            t.line = line;
            tokens.push_back(t);
            continue;
        }
        if (c == '"') {
            size_t start = i;
            ++i;
            while (i < n && _src[i] != '"') {
                if (_src[i] == '\\' && i + 1 < n) {
                    i += 2;
                    continue;
                }
                if (_src[i] == '\n') {
                    ++line;
                }
                ++i;
            }
            if (i >= n) {
                throw ParseError(_fileName, line, "文字列リテラルが閉じていない");
            }
            ++i; // 終端の "
            Token t;
            t.kind = TokenKind::StringLiteral;
            t.text = _src.substr(start, i - start);
            t.line = line;
            tokens.push_back(t);
            continue;
        }
        if (c == '\'') {
            size_t start = i;
            ++i;
            while (i < n && _src[i] != '\'') {
                if (_src[i] == '\\' && i + 1 < n) {
                    i += 2;
                    continue;
                }
                ++i;
            }
            if (i >= n) {
                throw ParseError(_fileName, line, "文字リテラルが閉じていない");
            }
            ++i;
            Token t;
            t.kind = TokenKind::CharLiteral;
            t.text = _src.substr(start, i - start);
            t.line = line;
            tokens.push_back(t);
            continue;
        }

        // 2文字記号(:: -> ->* だけこの用途では必要)
        static const char* kTwoCharPuncts[] = {"::", "->"};
        bool matchedTwo = false;
        for (const char* p : kTwoCharPuncts) {
            if (i + 1 < n && c == p[0] && _src[i + 1] == p[1]) {
                Token t;
                t.kind = TokenKind::Punct;
                t.text = std::string(p);
                t.line = line;
                tokens.push_back(t);
                i += 2;
                matchedTwo = true;
                break;
            }
        }
        if (matchedTwo) {
            continue;
        }

        // それ以外は1文字記号としてそのまま渡す
        Token t;
        t.kind = TokenKind::Punct;
        t.text = std::string(1, c);
        t.line = line;
        tokens.push_back(t);
        ++i;
    }

    Token end;
    end.kind = TokenKind::End;
    end.line = line;
    tokens.push_back(end);
    return tokens;
}

} // namespace ReflectionCodeGen

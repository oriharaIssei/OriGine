#include "Parser.h"

#include <cctype>
#include <optional>

namespace ReflectionCodeGen {

namespace {

using TokenVec = std::vector<Token>;

/// <summary>1つ以上のトークン列を線形走査するための小さなカーソル。</summary>
struct Cursor {
    const TokenVec* tokens;
    size_t pos;
    std::string fileName;

    const Token& Cur() const { return (*tokens)[pos]; }
    bool AtEnd() const { return Cur().kind == TokenKind::End; }
    bool Is(const char* _text) const { return Cur().kind != TokenKind::End && Cur().text == _text; }
    bool IsIdent(const char* _text) const { return Cur().kind == TokenKind::Identifier && Cur().text == _text; }

    void Expect(const char* _text) {
        if (!Is(_text)) {
            throw ParseError(fileName, Cur().line,
                "'" + std::string(_text) + "' を期待したが '" + Cur().text + "' だった");
        }
        ++pos;
    }
};

/// <summary>_openPos が指す '(' に対応する ')' の添字を返す。</summary>
size_t FindMatchingParen(const TokenVec& _tokens, size_t _openPos, const std::string& _fileName) {
    int depth = 0;
    for (size_t i = _openPos; i < _tokens.size(); ++i) {
        if (_tokens[i].kind == TokenKind::End) {
            break;
        }
        if (_tokens[i].text == "(") {
            ++depth;
        } else if (_tokens[i].text == ")") {
            --depth;
            if (depth == 0) {
                return i;
            }
        }
    }
    throw ParseError(_fileName, _tokens[_openPos].line, "'(' に対応する ')' が見つからない");
}

/// <summary>_openPos が指す '{' に対応する '}' の添字を返す。</summary>
size_t FindMatchingBrace(const TokenVec& _tokens, size_t _openPos, const std::string& _fileName) {
    int depth = 0;
    for (size_t i = _openPos; i < _tokens.size(); ++i) {
        if (_tokens[i].kind == TokenKind::End) {
            break;
        }
        if (_tokens[i].text == "{") {
            ++depth;
        } else if (_tokens[i].text == "}") {
            --depth;
            if (depth == 0) {
                return i;
            }
        }
    }
    throw ParseError(_fileName, _tokens[_openPos].line, "'{' に対応する '}' が見つからない");
}

/// <summary>[_from, _to) の範囲に、テキストが _needle と一致する識別子トークンがあるか。</summary>
bool ContainsIdentifier(const TokenVec& _tokens, size_t _from, size_t _to, const char* _needle) {
    for (size_t i = _from; i < _to && i < _tokens.size(); ++i) {
        if (_tokens[i].kind == TokenKind::Identifier && _tokens[i].text == _needle) {
            return true;
        }
    }
    return false;
}

/// <summary>
/// [_from, _to) の範囲に、条件付きコンパイルの前処理行( #if / #ifdef / #ifndef )が
/// あるか。Q14: 注釈付きの型では禁止し、見つかったらエラーで止める。
/// </summary>
void CheckNoConditionalDirectives(const TokenVec& _tokens, size_t _from, size_t _to,
    const std::string& _fileName, const std::string& _typeName) {
    for (size_t i = _from; i < _to && i < _tokens.size(); ++i) {
        const Token& t = _tokens[i];
        if (t.kind != TokenKind::Punct || t.text.empty() || t.text[0] != '#') {
            continue;
        }
        // トークナイザは前処理行を "#..." のまるごと1トークンにしている
        std::string directive = t.text.substr(1);
        while (!directive.empty() && (directive.front() == ' ' || directive.front() == '\t')) {
            directive.erase(directive.begin());
        }
        bool isConditional = directive.rfind("if", 0) == 0 // "if" / "ifdef" / "ifndef"
                              && (directive.size() == 2 || !std::isalnum(static_cast<unsigned char>(directive[2])));
        if (isConditional) {
            throw ParseError(_fileName, t.line,
                "注釈付き型 '" + _typeName + "' の中で条件付きコンパイル(" + t.text +
                    ") が使われている。オフセットが構成ごとに変わってしまうため禁止"
                    "(docs/plans/phase-03c-design.md 決定9)");
        }
    }
}

struct PendingFieldAttr {
    std::optional<std::string> jsonKeyOverride;
    bool noSave = false;
    int sourceLine = 0;
};

/// <summary>ORIGINE_FIELD の引数を解釈する。既に '(' の直後を指している状態で呼ぶ。</summary>
PendingFieldAttr ParseFieldAttrArgs(Cursor& _c) {
    PendingFieldAttr attr;
    attr.sourceLine = _c.Cur().line;

    if (_c.Is(")")) {
        throw ParseError(_c.fileName, _c.Cur().line, "ORIGINE_FIELD() の引数が空。no_save か json=\"...\" を指定すること");
    }

    while (true) {
        if (_c.IsIdent("no_save")) {
            attr.noSave = true;
            ++_c.pos;
        } else if (_c.IsIdent("json")) {
            ++_c.pos;
            _c.Expect("=");
            if (_c.Cur().kind != TokenKind::StringLiteral) {
                throw ParseError(_c.fileName, _c.Cur().line, "ORIGINE_FIELD(json=...) には文字列リテラルを渡すこと");
            }
            std::string raw = _c.Cur().text; // ダブルクォート込み
            if (raw.size() >= 2) {
                raw = raw.substr(1, raw.size() - 2);
            }
            attr.jsonKeyOverride = raw;
            ++_c.pos;
        } else {
            throw ParseError(_c.fileName, _c.Cur().line, "ORIGINE_FIELD の引数 '" + _c.Cur().text + "' を認識できない");
        }

        if (_c.Is(",")) {
            ++_c.pos;
            continue;
        }
        break;
    }
    return attr;
}

/// <summary>
/// 1文の走査結果。関数/コンストラクタ等なら isField=false。
/// </summary>
struct StatementResult {
    bool isField = false;
    std::string name;
    std::string typeSignature;
    std::vector<std::string> typeTokens;
    int sourceLine = 0;
};

/// <summary>
/// トークン列をつなげて型シグネチャ文字列を作る(空白なし)。
/// 比較用の正規化がこれだけで足りるのは、対象10型の型名がテンプレート引数を含めても
/// 空白の有無で意味が変わらない単純な形だから。
/// </summary>
std::string JoinTokens(const TokenVec& _tokens, size_t _from, size_t _to) {
    std::string s;
    for (size_t i = _from; i < _to; ++i) {
        s += _tokens[i].text;
    }
    return s;
}

/// <summary>[_from, _to) の個々のトークンのテキストを、連結せずそのまま並べて返す。</summary>
std::vector<std::string> TokenTexts(const TokenVec& _tokens, size_t _from, size_t _to) {
    std::vector<std::string> out;
    out.reserve(_to - _from);
    for (size_t i = _from; i < _to; ++i) {
        out.push_back(_tokens[i].text);
    }
    return out;
}

/// <summary>
/// prefixTokens([_from, _to))から「フィールド名」と「型トークン範囲」を求める。
/// 規則: 直近の最上位 '=' があればその直前の識別子が名前、無ければ範囲内最後の識別子が名前。
/// </summary>
void ExtractNameAndType(const TokenVec& _tokens, size_t _from, size_t _to, const std::string& _fileName,
    std::string& _outName, size_t& _outTypeFrom, size_t& _outTypeTo) {
    size_t eqPos = std::string::npos;
    for (size_t i = _from; i < _to; ++i) {
        if (_tokens[i].kind == TokenKind::Punct && _tokens[i].text == "=") {
            eqPos = i; // 最後の '=' を使う(複数出ることは無い想定だが安全側に倒す)
        }
    }
    size_t searchEnd = (eqPos == std::string::npos) ? _to : eqPos;

    size_t nameIdx = std::string::npos;
    for (size_t i = _from; i < searchEnd; ++i) {
        if (_tokens[i].kind == TokenKind::Identifier) {
            nameIdx = i;
        }
    }
    if (nameIdx == std::string::npos) {
        int line = (_from < _tokens.size()) ? _tokens[_from].line : 0;
        throw ParseError(_fileName, line, "フィールド宣言からメンバ名を特定できない");
    }
    _outName    = _tokens[nameIdx].text;
    _outTypeFrom = _from;
    _outTypeTo   = nameIdx;
}

/// <summary>
/// 1つのメンバ宣言(フィールド or 関数/コンストラクタ/デストラクタ)を読み飛ばし、
/// フィールドなら中身を返す。呼び出し時点で _c は宣言の先頭を指している。
///
/// フィールドか関数かの判定基準: トップレベルの '=' より前に '(' が現れるかどうか。
/// 関数の仮引数リストは名前の直後、'=' より必ず前に来る。フィールドの初期化子に
/// 出てくる '('(例: "Matrix4x4 worldMat = MakeMatrix4x4::Identity();"、
/// "float aspectRatio = (float)16 / 9;" のようなキャストや呼び出し)は必ず '=' の
/// 後に来る。対象10型に直接初期化 "Type name(args);"(= を伴わない)は無いため、
/// この基準だけで曖昧さなく判定できる。
/// </summary>
StatementResult ScanMemberStatement(Cursor& _c) {
    const TokenVec& tokens = *_c.tokens;
    size_t start = _c.pos;
    int startLine = _c.Cur().line;

    size_t i = start;
    bool sawEquals = false;
    bool isFunction = false;
    size_t nameBoundary = static_cast<size_t>(-1); // 名前抽出に使う範囲の終端
    size_t cursorAfter = static_cast<size_t>(-1);

    while (true) {
        if (i >= tokens.size() || tokens[i].kind == TokenKind::End) {
            throw ParseError(_c.fileName, startLine, "メンバ宣言が ';' で終わらないままファイル末尾に達した");
        }
        const Token& t = tokens[i];

        if (t.kind == TokenKind::Punct && t.text == "=" && !sawEquals) {
            sawEquals = true;
            if (nameBoundary == static_cast<size_t>(-1)) {
                nameBoundary = i;
            }
            ++i;
            continue;
        }
        if (t.kind == TokenKind::Punct && t.text == "(") {
            if (!sawEquals) {
                isFunction = true;
                break; // i はこの '(' を指したまま抜ける
            }
            i = FindMatchingParen(tokens, i, _c.fileName) + 1;
            continue;
        }
        if (t.kind == TokenKind::Punct && t.text == "{") {
            if (nameBoundary == static_cast<size_t>(-1)) {
                nameBoundary = i; // "Type name{...};" のような '=' 無しブレース初期化
            }
            i = FindMatchingBrace(tokens, i, _c.fileName) + 1;
            continue;
        }
        if (t.kind == TokenKind::Punct && t.text == ";") {
            if (nameBoundary == static_cast<size_t>(-1)) {
                nameBoundary = i;
            }
            cursorAfter = i + 1;
            break;
        }
        if (t.kind == TokenKind::Punct && t.text == "}") {
            // 対応する '{' を経由せずにここへ来たということは、囲んでいる型の閉じ括弧に
            // ぶつかったということ。';' を書き忘れた壊れた宣言(例: "int a = 0" のまま
            // 次が "};" )を捕まえるためのガード。
            throw ParseError(_c.fileName, startLine, "宣言が ';' で終わる前に閉じ括弧 '}' に達した(';' の書き忘れの可能性)");
        }
        ++i;
    }

    if (isFunction) {
        // 関数/コンストラクタ/デストラクタ/演算子オーバーロードの類。
        size_t closeParen = FindMatchingParen(tokens, i, _c.fileName);
        size_t p = closeParen + 1;
        // 修飾子(const/override/noexcept 等)、コンストラクタ初期化子リスト、本体を
        // 読み飛ばす。'(' が出るたびに対応する ')' までまとめて飛ばすので、
        // "scale(_scale), rotate(_rotate) {}" のような初期化子リストも
        // 個々の文法を知らなくても正しく飛ばせる。
        while (true) {
            if (p >= tokens.size() || tokens[p].kind == TokenKind::End) {
                throw ParseError(_c.fileName, startLine, "関数/コンストラクタの本体が見つからないままファイル末尾に達した");
            }
            const Token& pt = tokens[p];
            if (pt.kind == TokenKind::Punct && pt.text == "}") {
                throw ParseError(_c.fileName, startLine, "関数/コンストラクタ宣言が ';' や本体で終わる前に閉じ括弧 '}' に達した");
            }
            if (pt.kind == TokenKind::Punct && pt.text == "(") {
                p = FindMatchingParen(tokens, p, _c.fileName) + 1;
                continue;
            }
            if (pt.kind == TokenKind::Punct && pt.text == "{") {
                size_t close = FindMatchingBrace(tokens, p, _c.fileName);
                _c.pos = close + 1;
                StatementResult r;
                r.isField    = false;
                r.sourceLine = startLine;
                return r;
            }
            if (pt.kind == TokenKind::Punct && pt.text == ";") {
                _c.pos = p + 1;
                StatementResult r;
                r.isField    = false;
                r.sourceLine = startLine;
                return r;
            }
            ++p;
        }
    }

    // フィールド。
    std::string name;
    size_t typeFrom = 0, typeTo = 0;
    ExtractNameAndType(tokens, start, nameBoundary, _c.fileName, name, typeFrom, typeTo);
    _c.pos = cursorAfter;
    StatementResult r;
    r.isField       = true;
    r.name           = name;
    r.typeSignature  = JoinTokens(tokens, typeFrom, typeTo);
    r.typeTokens     = TokenTexts(tokens, typeFrom, typeTo);
    r.sourceLine     = startLine;
    return r;
}

/// <summary>friend 宣言や using/typedef を次の ';' まで読み飛ばす。</summary>
void SkipToSemicolon(Cursor& _c) {
    const TokenVec& tokens = *_c.tokens;
    size_t i = _c.pos;
    int parenDepth = 0;
    while (true) {
        if (i >= tokens.size() || tokens[i].kind == TokenKind::End) {
            throw ParseError(_c.fileName, _c.Cur().line, "';' が見つからないままファイル末尾に達した");
        }
        const Token& t = tokens[i];
        if (t.text == "(") {
            ++parenDepth;
        } else if (t.text == ")") {
            --parenDepth;
        } else if (t.text == ";" && parenDepth == 0) {
            _c.pos = i + 1;
            return;
        }
        ++i;
    }
}

void ParseEnum(Cursor& _c, std::vector<EnumInfo>& _outEnums) {
    _c.Expect("enum");
    if (_c.Is("class") || _c.Is("struct")) {
        ++_c.pos;
    }
    if (_c.Cur().kind != TokenKind::Identifier) {
        throw ParseError(_c.fileName, _c.Cur().line, "無名の enum には対応していない");
    }
    std::string name = _c.Cur().text;
    ++_c.pos;

    std::string underlying = "int";
    if (_c.Is(":")) {
        ++_c.pos;
        std::string u;
        while (!_c.Is("{") && !_c.Is(";")) {
            if (_c.AtEnd()) {
                throw ParseError(_c.fileName, _c.Cur().line, "enum の下地型指定が終わらない");
            }
            u += _c.Cur().text;
            ++_c.pos;
        }
        underlying = u;
    }

    if (_c.Is(";")) {
        // 前方宣言。中身が無いので登録しない。
        ++_c.pos;
        return;
    }
    if (!_c.Is("{")) {
        throw ParseError(_c.fileName, _c.Cur().line, "enum の後に '{' か ';' を期待した");
    }
    size_t close = FindMatchingBrace(*_c.tokens, _c.pos, _c.fileName);
    _c.pos = close + 1;
    if (!_c.Is(";")) {
        throw ParseError(_c.fileName, _c.Cur().line, "enum 定義の後に ';' が無い");
    }
    ++_c.pos;

    EnumInfo info;
    info.name               = name;
    info.underlyingTypeText = underlying;
    _outEnums.push_back(info);
}

/// <summary>
/// struct/class 定義を1つ読む。ORIGINE_COMPONENT() を含んでいれば注釈付き型として
/// public フィールドを収集し _outResult.types に足す。含んでいなければ、内容は
/// 解釈せず中括弧の対応だけでまるごと読み飛ばす(入れ子の ConstantBuffer 等)。
/// </summary>
void ParseClassOrStruct(Cursor& _c, const std::string& _headerIncludePath, FileParseResult& _outResult) {
    const TokenVec& tokens = *_c.tokens;
    bool isStructKeyword = _c.Is("struct");
    _c.Expect(isStructKeyword ? "struct" : "class");

    // OriGineApi.h の ORIGINE_API(dllexport/dllimport注釈、Phase 4 4D)は実コンパイラには
    // 透過的なマクロだが、このパーサは実プリプロセッサを通さずトークン列をそのまま読むため、
    // `struct ORIGINE_API Foo` の ORIGINE_API を型名と取り違えて壊れる。
    // 対象型(targets.txtに列挙された型。今のところ Transform.h のみ)にこの注釈が付いた実例が
    // あるため、struct/class キーワード直後の "ORIGINE_API" だけを読み飛ばす。
    if (_c.Is("ORIGINE_API")) {
        ++_c.pos;
    }

    if (_c.Cur().kind != TokenKind::Identifier) {
        throw ParseError(_c.fileName, _c.Cur().line, "struct/class の名前が識別子ではない");
    }
    std::string className = _c.Cur().text;
    ++_c.pos;

    if (_c.Is("final")) {
        ++_c.pos;
    }

    if (_c.Is(";")) {
        // 前方宣言。読み飛ばして終わり。
        ++_c.pos;
        return;
    }

    if (_c.Is(":")) {
        // 基底クラス節: (public|private|protected)? (virtual)? Name (, ...)* まで読み飛ばし、
        // 個々の継承元の中身には立ち入らない(D5: 平坦化は生成側ではなく offsetof に任せる)。
        ++_c.pos;
        while (!_c.Is("{")) {
            if (_c.AtEnd()) {
                throw ParseError(_c.fileName, _c.Cur().line, "基底クラス節が '{' で終わらない");
            }
            ++_c.pos;
        }
    }

    if (!_c.Is("{")) {
        throw ParseError(_c.fileName, _c.Cur().line, "struct/class '" + className + "' の後に '{' を期待した");
    }
    size_t bodyOpen  = _c.pos;
    size_t bodyClose = FindMatchingBrace(tokens, bodyOpen, _c.fileName);

    bool hasComponentAnnotation = ContainsIdentifier(tokens, bodyOpen, bodyClose, "ORIGINE_COMPONENT");
    bool hasStructAnnotation    = ContainsIdentifier(tokens, bodyOpen, bodyClose, "ORIGINE_STRUCT");

    if (hasComponentAnnotation && hasStructAnnotation) {
        throw ParseError(_c.fileName, tokens[bodyOpen].line,
            "型 '" + className + "' に ORIGINE_COMPONENT() と ORIGINE_STRUCT() の両方が付いている"
            "(コンポーネントであると同時に入れ子専用の構造体、という状態は無い)");
    }
    bool annotated = hasComponentAnnotation || hasStructAnnotation;

    if (!annotated) {
        // 反映対象外の型。中身は一切解釈せず、対応する '}' の次まで読み飛ばすだけにする。
        // (ConstantBuffer のような operator= オーバーロード等、この関数が知らない構文を
        //  含んでいても問題にならないようにするための意図的な設計。)
        _c.pos = bodyClose + 1;
        if (!_c.Is(";")) {
            throw ParseError(_c.fileName, _c.Cur().line, "struct/class '" + className + "' の定義の後に ';' が無い");
        }
        ++_c.pos;
        return;
    }

    CheckNoConditionalDirectives(tokens, bodyOpen, bodyClose, _c.fileName, className);

    TypeInfo typeInfo;
    typeInfo.name               = className;
    typeInfo.headerIncludePath = _headerIncludePath;
    typeInfo.isComponent        = hasComponentAnnotation;

    bool currentAccessIsPublic = isStructKeyword; // struct既定はpublic、classは既定private
    std::optional<PendingFieldAttr> pendingAttr;

    _c.pos = bodyOpen + 1;
    while (_c.pos < bodyClose) {
        const Token& t = _c.Cur();

        if (t.kind == TokenKind::Punct && !t.text.empty() && t.text[0] == '#') {
            // 前処理行。CheckNoConditionalDirectives で既に条件付きは弾いてあるので、
            // #pragma 等の残りはそのまま読み飛ばしてよい。
            ++_c.pos;
            continue;
        }

        if (t.kind == TokenKind::Punct && t.text == ";") {
            // 空文(例: "void Finalize() {};" のように関数定義の直後に付く ';')。
            ++_c.pos;
            continue;
        }

        if (t.kind == TokenKind::Identifier && (t.text == "public" || t.text == "private" || t.text == "protected")) {
            size_t next = _c.pos + 1;
            if (next < tokens.size() && tokens[next].kind == TokenKind::Punct && tokens[next].text == ":") {
                if (pendingAttr) {
                    throw ParseError(_c.fileName, t.line, "ORIGINE_FIELD の直後がフィールド宣言ではない(アクセス指定子が来た)");
                }
                currentAccessIsPublic = (t.text == "public");
                _c.pos                = next + 1;
                continue;
            }
            // "public" 等がラベルでない使われ方をするのはこの文脈では想定していない。
            throw ParseError(_c.fileName, t.line, "'" + t.text + "' の後に ':' が無い(アクセス指定子として読めない)");
        }

        if (t.kind == TokenKind::Identifier && t.text == "friend") {
            if (pendingAttr) {
                throw ParseError(_c.fileName, t.line, "ORIGINE_FIELD の直後がフィールド宣言ではない(friend 宣言が来た)");
            }
            SkipToSemicolon(_c);
            continue;
        }

        if (t.kind == TokenKind::Identifier && (t.text == "using" || t.text == "typedef")) {
            if (pendingAttr) {
                throw ParseError(_c.fileName, t.line, "ORIGINE_FIELD の直後がフィールド宣言ではない(using/typedef が来た)");
            }
            SkipToSemicolon(_c);
            continue;
        }

        if (t.kind == TokenKind::Identifier && t.text == "enum") {
            if (pendingAttr) {
                throw ParseError(_c.fileName, t.line, "ORIGINE_FIELD の直後がフィールド宣言ではない(enum が来た)");
            }
            // 入れ子の enum。対象10型では使っていないが、一般化のため素通りできるようにする。
            std::vector<EnumInfo> dummy;
            ParseEnum(_c, dummy);
            for (auto& e : dummy) {
                _outResult.enums.push_back(e);
            }
            continue;
        }

        if (t.kind == TokenKind::Identifier && (t.text == "struct" || t.text == "class")) {
            if (pendingAttr) {
                throw ParseError(_c.fileName, t.line, "ORIGINE_FIELD の直後がフィールド宣言ではない(入れ子の型が来た)");
            }
            size_t before = _outResult.types.size();
            ParseClassOrStruct(_c, _headerIncludePath, _outResult);
            if (_outResult.types.size() != before) {
                throw ParseError(_c.fileName, t.line,
                    "型 '" + className + "' の中に注釈付きの入れ子型がある。対応していない");
            }
            continue;
        }

        if (t.kind == TokenKind::Identifier && t.text == "ORIGINE_COMPONENT") {
            if (pendingAttr) {
                throw ParseError(_c.fileName, t.line, "ORIGINE_FIELD の直後が ORIGINE_COMPONENT() だった");
            }
            ++_c.pos;
            _c.Expect("(");
            _c.Expect(")");
            _c.Expect(";");
            continue;
        }

        if (t.kind == TokenKind::Identifier && t.text == "ORIGINE_STRUCT") {
            if (pendingAttr) {
                throw ParseError(_c.fileName, t.line, "ORIGINE_FIELD の直後が ORIGINE_STRUCT() だった");
            }
            ++_c.pos;
            _c.Expect("(");
            _c.Expect(")");
            _c.Expect(";");
            continue;
        }

        if (t.kind == TokenKind::Identifier && t.text == "ORIGINE_FIELD") {
            if (pendingAttr) {
                throw ParseError(_c.fileName, t.line, "ORIGINE_FIELD が連続している(1フィールドにつき1回まで)");
            }
            ++_c.pos;
            _c.Expect("(");
            PendingFieldAttr attr = ParseFieldAttrArgs(_c);
            _c.Expect(")");
            _c.Expect(";");
            pendingAttr = attr;
            continue;
        }

        // ここまでで拾えなかったものはフィールドか関数/コンストラクタ/デストラクタ。
        StatementResult stmt = ScanMemberStatement(_c);

        if (!stmt.isField) {
            if (pendingAttr) {
                throw ParseError(_c.fileName, pendingAttr->sourceLine,
                    "ORIGINE_FIELD の直後が関数/コンストラクタ宣言だった(フィールドの直前に置くこと)");
            }
            continue;
        }

        if (!currentAccessIsPublic) {
            if (pendingAttr) {
                throw ParseError(_c.fileName, pendingAttr->sourceLine,
                    "ORIGINE_FIELD が public 以外のフィールドに付いている(反映対象は public のみ)");
            }
            continue; // 非publicは黙って反映対象から外す(設計どおり)
        }

        FieldInfo field;
        field.name          = stmt.name;
        field.typeSignature  = stmt.typeSignature;
        field.typeTokens     = stmt.typeTokens;
        field.sourceLine     = stmt.sourceLine;
        if (pendingAttr) {
            field.noSave = pendingAttr->noSave;
            field.jsonKey = pendingAttr->jsonKeyOverride
                                ? *pendingAttr->jsonKeyOverride
                                : stmt.name;
        } else {
            field.jsonKey = stmt.name;
        }
        if (!pendingAttr || !pendingAttr->jsonKeyOverride) {
            // 既定規則: メンバ名から末尾の '_' を1つだけ除く(D4)。
            if (!field.jsonKey.empty() && field.jsonKey.back() == '_') {
                field.jsonKey.pop_back();
            }
        }
        typeInfo.fields.push_back(field);
        pendingAttr.reset();
    }

    if (pendingAttr) {
        throw ParseError(_c.fileName, pendingAttr->sourceLine, "ORIGINE_FIELD の直後にフィールドが無いまま型定義が終わった");
    }

    if (typeInfo.fields.empty()) {
        throw ParseError(_c.fileName, tokens[bodyOpen].line,
            "ORIGINE_COMPONENT() は付いているが public フィールドが1つも見つからない型 '" + className + "'");
    }

    _outResult.types.push_back(typeInfo);

    _c.pos = bodyClose + 1;
    if (!_c.Is(";")) {
        throw ParseError(_c.fileName, _c.Cur().line, "struct/class '" + className + "' の定義の後に ';' が無い");
    }
    ++_c.pos;
}

} // namespace

FileParseResult ParseFile(const std::string& _src, const std::string& _fileName, const std::string& _headerIncludePath) {
    std::string stripped = StripComments(_src);
    std::vector<Token> tokens = Tokenize(stripped, _fileName);

    Cursor c{&tokens, 0, _fileName};
    FileParseResult result;

    while (!c.AtEnd()) {
        const Token& t = c.Cur();

        if (t.kind == TokenKind::Punct && !t.text.empty() && t.text[0] == '#') {
            ++c.pos;
            continue;
        }
        if (t.kind == TokenKind::Punct && t.text == "}") {
            // namespace の閉じ括弧。
            ++c.pos;
            continue;
        }
        if (t.kind == TokenKind::Punct && t.text == ";") {
            ++c.pos;
            continue;
        }

        if (t.kind == TokenKind::Identifier && t.text == "namespace") {
            ++c.pos;
            while (c.Cur().kind == TokenKind::Identifier || c.Is("::")) {
                ++c.pos;
            }
            c.Expect("{");
            continue;
        }
        if (t.kind == TokenKind::Identifier && t.text == "enum") {
            ParseEnum(c, result.enums);
            continue;
        }
        if (t.kind == TokenKind::Identifier && (t.text == "struct" || t.text == "class")) {
            ParseClassOrStruct(c, _headerIncludePath, result);
            continue;
        }
        if (t.kind == TokenKind::Identifier && (t.text == "using" || t.text == "typedef")) {
            SkipToSemicolon(c);
            continue;
        }
        if (t.kind == TokenKind::Identifier
            && (t.text == "ORIGINE_COMPONENT" || t.text == "ORIGINE_STRUCT" || t.text == "ORIGINE_FIELD")) {
            throw ParseError(_fileName, t.line, t.text + " が型定義の外で使われている");
        }

        // それ以外(namespace スコープの free function 定義など)は読み飛ばす。
        StatementResult stmt = ScanMemberStatement(c);
        (void)stmt;
    }

    return result;
}

} // namespace ReflectionCodeGen

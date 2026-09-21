// ============================================================================
// ReflectionCodeGen: 型ディスクリプタ生成ツール(Phase 3C, C-3)。
//
// 使い方:
//   ReflectionCodeGen.exe --code-root <dir> --manifest <targets.txt> --out <dir>
//     targets.txt に列挙されたヘッダ(--code-root からの相対パス、1行1本、
//     '#' で始まる行はコメント)を読み、ORIGINE_COMPONENT() が付いた型を集めて
//     <out>/ComponentDescriptors.generated.h / .cpp を書く。
//     内容ハッシュが既存ファイルと同じなら書き込まない(タイムスタンプも変えない)。
//
//   ReflectionCodeGen.exe --selftest
//     ツール自身の自己診断: (1) 壊れた入力で非0終了になること、
//     (2) 同じ入力で2回連続生成すると2回目は書き込みが起きないこと、を確かめる。
//
// このツールはエンジンのヘッダに依存しない(単体でビルドできる)。標準ライブラリのみ使う。
// ============================================================================

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "Classifier.h"
#include "Generator.h"
#include "Hash.h"
#include "Model.h"
#include "Parser.h"
#include "Tokenizer.h"

namespace fs = std::filesystem;
using namespace ReflectionCodeGen;

namespace {

std::string ReadFileOrThrow(const fs::path& _path) {
    std::ifstream f(_path, std::ios::binary);
    if (!f) {
        throw std::runtime_error("ファイルを開けない: " + _path.string());
    }
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

/// <summary>
/// _path に既に _content と同じ内容が書かれていれば何もしない(タイムスタンプも
/// 更新しない)。違えば書き込む。戻り値は「実際に書き込んだか」。
/// </summary>
bool WriteIfChanged(const fs::path& _path, const std::string& _content) {
    if (fs::exists(_path)) {
        std::string existing = ReadFileOrThrow(_path);
        if (Fnv1a64(existing) == Fnv1a64(_content)) {
            return false;
        }
    }
    fs::create_directories(_path.parent_path());
    std::ofstream f(_path, std::ios::binary | std::ios::trunc);
    if (!f) {
        throw std::runtime_error("ファイルを書き込めない: " + _path.string());
    }
    f << _content;
    return true;
}

std::vector<std::string> ReadManifest(const fs::path& _manifestPath) {
    std::ifstream f(_manifestPath);
    if (!f) {
        throw std::runtime_error("マニフェストを開けない: " + _manifestPath.string());
    }
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(f, line)) {
        // 前後の空白除去
        size_t b = line.find_first_not_of(" \t\r\n");
        if (b == std::string::npos) {
            continue;
        }
        size_t e = line.find_last_not_of(" \t\r\n");
        line     = line.substr(b, e - b + 1);
        if (line.empty() || line[0] == '#') {
            continue;
        }
        lines.push_back(line);
    }
    return lines;
}

/// <summary>
/// 実際の生成処理。--selftest からも(一時ディレクトリに向けて)再利用する。
/// 例外は呼び出し元で捕まえる。
/// </summary>
/// <returns>1つでもファイルを書き込んだら true</returns>
bool RunGeneration(const fs::path& _codeRoot, const fs::path& _manifestPath, const fs::path& _outDir, bool _verbose) {
    std::vector<std::string> headerRelPaths = ReadManifest(_manifestPath);
    if (headerRelPaths.empty()) {
        throw std::runtime_error("マニフェストに対象ヘッダが1つも無い: " + _manifestPath.string());
    }

    std::vector<TypeInfo> allTypes;
    std::vector<EnumInfo> allEnums;

    for (const std::string& rel : headerRelPaths) {
        fs::path headerPath = _codeRoot / rel;
        std::string src      = ReadFileOrThrow(headerPath);
        // 生成 .cpp の #include はスラッシュ区切りで統一する(Windows でも forward slash はOK)。
        std::string includePath = rel;
        for (char& ch : includePath) {
            if (ch == '\\') {
                ch = '/';
            }
        }

        FileParseResult r = ParseFile(src, headerPath.string(), includePath);
        if (r.types.empty()) {
            throw std::runtime_error(headerPath.string() + ": ORIGINE_COMPONENT()/ORIGINE_STRUCT() が付いた型が見つからない(manifest に載せる意味が無い)");
        }
        for (auto& t : r.types) {
            allTypes.push_back(std::move(t));
        }
        for (auto& e : r.enums) {
            allEnums.push_back(std::move(e));
        }
        if (_verbose) {
            std::cout << "  parsed " << headerPath.string() << " -> " << r.types.size() << " type(s), "
                      << r.enums.size() << " enum(s)" << std::endl;
        }
    }

    // 入れ子構造体(ORIGINE_STRUCT())の名前一覧。発見順(=ファイルをmanifest順に読み、
    // 各ファイル内は上から)で並べる。この並び順が、Generatorが組む入れ子構造体専用の
    // TypeDesc表の並び順、ひいてはFieldDesc::nestedTypeIndex_の意味そのものになる。
    std::vector<std::string> structNames;
    for (const auto& t : allTypes) {
        if (!t.isComponent) {
            structNames.push_back(t.name);
        }
    }

    // 分類(enum registry / structNames が全ファイル分揃ってから行う)
    std::vector<ClassifiedType> classified;
    classified.reserve(allTypes.size());
    for (const auto& t : allTypes) {
        ClassifiedType ct;
        ct.name               = t.name;
        ct.headerIncludePath = t.headerIncludePath;
        ct.isComponent        = t.isComponent;
        ct.fields.reserve(t.fields.size());
        for (const auto& f : t.fields) {
            ClassifiedField cf;
            cf.name              = f.name;
            cf.jsonKey           = f.jsonKey;
            cf.noSave            = f.noSave;
            cf.enumIndex         = FindEnumIndex(f.typeSignature, allEnums);
            cf.nestedStructIndex = FindNestedStructIndex(f.typeTokens, structNames, t.headerIncludePath, f.sourceLine);
            if (cf.enumIndex >= 0 && cf.nestedStructIndex >= 0) {
                // enum名一致(完全一致)と構造体名一致(トークン一致)が同じフィールドで
                // 両方成立することは対象型では起こらない想定。起きたら「なんとなくどちらかを
                // 採用する」のではなく、想定外の入力として止める。
                throw std::runtime_error(t.headerIncludePath + ": フィールド '" + f.name +
                    "' が enum 判定と入れ子構造体判定の両方に一致した(想定外の型名の衝突)");
            }
            ct.fields.push_back(cf);
        }
        classified.push_back(ct);
    }

    GeneratedFiles gen = Generate(classified, allEnums);

    bool wroteHeader = WriteIfChanged(_outDir / "ComponentDescriptors.generated.h", gen.headerText);
    bool wroteCpp     = WriteIfChanged(_outDir / "ComponentDescriptors.generated.cpp", gen.cppText);

    if (_verbose) {
        std::cout << "  types=" << classified.size();
        size_t fieldCount = 0;
        for (auto& t : classified) fieldCount += t.fields.size();
        std::cout << " fields=" << fieldCount
                   << " wroteHeader=" << (wroteHeader ? "yes" : "no")
                   << " wroteCpp=" << (wroteCpp ? "yes" : "no") << std::endl;
    }

    return wroteHeader || wroteCpp;
}

int RunSelfTest() {
    std::cout << "ReflectionCodeGen --selftest" << std::endl;
    bool allOk = true;

    fs::path tmpRoot = fs::temp_directory_path() / "ReflectionCodeGenSelfTest";
    std::error_code ec;
    fs::remove_all(tmpRoot, ec);
    fs::create_directories(tmpRoot / "code" / "component");
    fs::create_directories(tmpRoot / "out");

    // --- ケース1: 壊れた入力は非0で止まる ---
    {
        std::ofstream f(tmpRoot / "code" / "component" / "Broken.h");
        f << "namespace OriGine {\n"
             "struct Broken {\n"
             "public:\n"
             "    ORIGINE_COMPONENT();\n"
             "    int a = 0\n" // ';' が無い壊れた入力
             "};\n"
             "} // namespace OriGine\n";
    }
    {
        std::ofstream f(tmpRoot / "manifest_broken.txt");
        f << "component/Broken.h\n";
    }

    bool threw = false;
    try {
        RunGeneration(tmpRoot / "code", tmpRoot / "manifest_broken.txt", tmpRoot / "out", false);
    } catch (const std::exception&) {
        threw = true;
    }
    if (threw) {
        std::cout << "  [PASS] 壊れた入力で例外(非0終了相当)になった" << std::endl;
    } else {
        std::cout << "  [FAIL] 壊れた入力なのに例外にならなかった" << std::endl;
        allOk = false;
    }

    // --- ケース2: 正常な入力で2回連続生成すると、2回目は書き込みが起きない ---
    {
        std::ofstream f(tmpRoot / "code" / "component" / "Good.h");
        f << "namespace OriGine {\n"
             "struct Good {\n"
             "public:\n"
             "    ORIGINE_COMPONENT();\n"
             "    float value = 0.0f;\n"
             "    bool flag = true;\n"
             "};\n"
             "} // namespace OriGine\n";
    }
    {
        std::ofstream f(tmpRoot / "manifest_good.txt");
        f << "component/Good.h\n";
    }

    bool firstWrote  = false;
    bool secondWrote = true;
    bool ok2         = true;
    try {
        firstWrote  = RunGeneration(tmpRoot / "code", tmpRoot / "manifest_good.txt", tmpRoot / "out", false);
        secondWrote = RunGeneration(tmpRoot / "code", tmpRoot / "manifest_good.txt", tmpRoot / "out", false);
    } catch (const std::exception& e) {
        std::cout << "  [FAIL] 正常な入力なのに例外: " << e.what() << std::endl;
        ok2 = false;
    }
    if (ok2) {
        if (firstWrote && !secondWrote) {
            std::cout << "  [PASS] 1回目は書き込み、2回目(内容不変)は書き込まなかった" << std::endl;
        } else {
            std::cout << "  [FAIL] 期待した書き込みパターンにならなかった(1回目=" << firstWrote
                       << " 2回目=" << secondWrote << ")" << std::endl;
            allOk = false;
        }
    } else {
        allOk = false;
    }

    fs::remove_all(tmpRoot, ec);

    std::cout << (allOk ? "ReflectionCodeGen --selftest: ALL PASS" : "ReflectionCodeGen --selftest: FAILED") << std::endl;
    return allOk ? 0 : 1;
}

void PrintUsage() {
    std::cerr << "usage: ReflectionCodeGen --code-root <dir> --manifest <targets.txt> --out <dir>\n"
                 "       ReflectionCodeGen --selftest\n";
}

} // namespace

int main(int argc, char** argv) {
    std::vector<std::string> args(argv + 1, argv + argc);

    if (args.size() == 1 && args[0] == "--selftest") {
        return RunSelfTest();
    }

    std::string codeRoot, manifest, outDir;
    bool verbose = false;
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--code-root" && i + 1 < args.size()) {
            codeRoot = args[++i];
        } else if (args[i] == "--manifest" && i + 1 < args.size()) {
            manifest = args[++i];
        } else if (args[i] == "--out" && i + 1 < args.size()) {
            outDir = args[++i];
        } else if (args[i] == "--verbose") {
            verbose = true;
        } else {
            std::cerr << "unknown argument: " << args[i] << std::endl;
            PrintUsage();
            return 1;
        }
    }

    if (codeRoot.empty() || manifest.empty() || outDir.empty()) {
        PrintUsage();
        return 1;
    }

    try {
        RunGeneration(fs::path(codeRoot), fs::path(manifest), fs::path(outDir), verbose);
    } catch (const std::exception& e) {
        std::cerr << "ReflectionCodeGen: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

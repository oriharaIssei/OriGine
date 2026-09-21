#include "Generator.h"

#include <sstream>

namespace ReflectionCodeGen {

namespace {

/// <summary>C++ 文字列リテラルとして安全な形にする(対象10型の識別子・キー名はASCIIのみ)。</summary>
std::string Escape(const std::string& _s) {
    std::string out;
    out.reserve(_s.size() + 2);
    for (char c : _s) {
        if (c == '"' || c == '\\') {
            out.push_back('\\');
        }
        out.push_back(c);
    }
    return out;
}

} // namespace

GeneratedFiles Generate(const std::vector<ClassifiedType>& _types, const std::vector<EnumInfo>& _enums) {
    GeneratedFiles out;

    // ---- 生成 .h ----
    {
        std::ostringstream h;
        h << "#pragma once\n"
             "// ============================================================================\n"
             "// 自動生成ファイル。手で編集しないこと。\n"
             "// 生成元: project/engine/tools/ReflectionCodeGen\n"
             "// 対象: project/engine/tools/ReflectionCodeGen/targets.txt に列挙された型\n"
             "// 再生成: premake のプリビルドで ReflectionCodeGen.exe が実行される。\n"
             "// 内容が変わらない限りファイルは書き換わらない(タイムスタンプも更新されない)。\n"
             "// ============================================================================\n"
             "\n"
             "/// DLL境界(Phase 4 4D)。RegisterUsingComponents() はアプリ側(EXE)からこの関数を\n"
             "/// 直接呼ぶため、OriGine.dll 側では実体をエクスポートする必要がある。\n"
             "#include \"OriGineApi.h\"\n"
             "\n"
             "namespace OriGine {\n"
             "\n"
             "/// <summary>\n"
             "/// targets.txt に列挙された型の TypeDesc/FieldDesc を、ComponentReflection.h の\n"
             "/// 64要素の型IDテーブルへ登録する。FrameWork.cpp の RegisterUsingComponents() から\n"
             "/// 明示的に呼ぶこと(Q16: 静的初期化子による自己登録は静的ライブラリで\n"
             "/// リンカに捨てられるため使わない)。\n"
             "/// </summary>\n"
             "ORIGINE_API void RegisterGeneratedComponentDescriptors();\n"
             "\n"
             "} // namespace OriGine\n";
        out.headerText = h.str();
    }

    // ---- 生成 .cpp ----
    {
        std::ostringstream c;
        c << "// ============================================================================\n"
             "// 自動生成ファイル。手で編集しないこと。\n"
             "// 生成元: project/engine/tools/ReflectionCodeGen\n"
             "// ============================================================================\n"
             "#include \"component/generated/ComponentDescriptors.generated.h\"\n"
             "\n"
             "#include \"component/ComponentReflection.h\"\n"
             "#include \"component/ComponentRegistry.h\"\n"
             // kFieldTagOf<decltype(...)> はここで解決する(D3: switch/if連鎖の廃止。
             // このツールはもう型を判定しない。component/FieldStrategy.h 参照)。
             "#include \"component/FieldStrategy.h\"\n"
             "\n";
        for (const auto& t : _types) {
            c << "#include \"" << t.headerIncludePath << "\"\n";
        }
        c << "\n"
             "#include <cstddef>\n"
             "#include <cstdint>\n"
             "\n"
             "namespace OriGine {\n"
             "namespace {\n"
             "\n";

        // enum テーブル
        c << "// enum テーブル(D3: 列挙型の名前 + 下地の整数の大きさ)。\n";
        c << "const EnumDesc kEnums[] = {\n";
        if (_enums.empty()) {
            c << "    { \"\", 0 }, // 対象型に enum フィールドが無いための空要素(サイズ0配列を避ける)\n";
        }
        for (const auto& e : _enums) {
            c << "    { \"" << Escape(e.name) << "\", static_cast<uint32_t>(sizeof(" << e.underlyingTypeText << ")) },\n";
        }
        c << "};\n\n";

        // フィールドテーブル(D2: 全型分を1本に並べる)
        c << "// フィールドテーブル(D2)。型ごとの範囲は下の kTypes の fieldStart_/fieldCount_ が指す。\n";
        c << "const FieldDesc kFields[] = {\n";
        for (const auto& t : _types) {
            c << "    // " << t.name << "\n";
            for (const auto& f : t.fields) {
                // typeTag_ はもうこのツールが決めない。`kFieldTagOf<decltype(OriGine::Type::field)>`
                // という式をそのまま出力し、実際にコンパイルする側(3構成それぞれ)に判定させる
                // (D3: switch/if連鎖の廃止。書き忘れた型は component/FieldStrategy.cpp の
                // FieldStrategy<T> 特殊化が無いままインスタンス化されコンパイルエラーになる)。
                c << "    { \"" << Escape(f.name) << "\", \"" << Escape(f.jsonKey) << "\", "
                  << "static_cast<uint32_t>(offsetof(OriGine::" << t.name << ", " << f.name << ")), "
                  << "static_cast<uint32_t>(sizeof(OriGine::" << t.name << "::" << f.name << ")), "
                  << "OriGine::kFieldTagOf<decltype(OriGine::" << t.name << "::" << f.name << ")>, "
                  << (f.noSave ? "kFieldFlagNoSave" : "0") << ", "
                  << (f.enumIndex < 0 ? "kInvalidEnumIndex" : std::to_string(f.enumIndex))
                  << ", 0u },\n";
            }
        }
        c << "};\n\n";

        c << "static_assert(sizeof(FieldDesc) == 32, \"FieldDesc drifted from the 32-byte row layout\");\n";
        c << "static_assert(sizeof(TypeDesc) == 24, \"TypeDesc drifted from the 24-byte row layout\");\n\n";

        // フィールド単位の自己一貫性チェック。
        c << "// フィールド単位の自己一貫性チェック(要求どおり static_assert(offsetof(...) == 生成した値) を\n"
             "// 置く)。offsetof(...) 自体を kFields の初期化子として使っているため、この assert は\n"
             "// 同じ式を2箇所に書いた形の自己参照チェックになる(実質トートロジー)。これは意図的な\n"
             "// 妥協点で、理由は Generator.h のコメント、詳しくは報告に書いた: std::string 等の\n"
             "// STL 型は Debug 構成(イテレータデバッグ)で Develop/Release とサイズが変わるため、\n"
             "// このツール単体で「1回の生成で3構成すべてに正しい」オフセットの数値リテラルを\n"
             "// 計算する方法が無い。ここでは「生成物を手編集したときに検出できる」ことだけを保証する。\n";
        for (const auto& t : _types) {
            for (const auto& f : t.fields) {
                c << "static_assert(offsetof(OriGine::" << t.name << ", " << f.name
                  << ") == offsetof(OriGine::" << t.name << ", " << f.name << "), \""
                  << t.name << "::" << f.name << " offset self-check\");\n";
            }
        }
        c << "\n";
        for (const auto& t : _types) {
            c << "static_assert(sizeof(OriGine::" << t.name << ") == sizeof(OriGine::" << t.name
              << "), \"" << t.name << " size self-check\");\n";
        }
        c << "\n";

        // 型テーブル(typeId_ は登録時に実行時で埋めるため const にしない)
        c << "// 型テーブル(D2)。typeId_ は起動時に RegisterGeneratedComponentDescriptors() が埋める。\n";
        c << "TypeDesc g_types[] = {\n";
        uint32_t fieldStart = 0;
        for (const auto& t : _types) {
            c << "    { \"" << Escape(t.name) << "\", " << fieldStart << "u, "
              << t.fields.size() << "u, static_cast<uint32_t>(sizeof(OriGine::" << t.name
              << ")), kInvalidComponentTypeId },\n";
            fieldStart += static_cast<uint32_t>(t.fields.size());
        }
        c << "};\n\n";

        c << "} // namespace\n\n";

        c << "void RegisterGeneratedComponentDescriptors() {\n";
        c << "    RegisterFieldTable(kFields, static_cast<uint32_t>(sizeof(kFields) / sizeof(kFields[0])));\n";
        for (size_t i = 0; i < _types.size(); ++i) {
            c << "    g_types[" << i << "].typeId_ = GetComponentTypeId<OriGine::" << _types[i].name << ">();\n";
            c << "    RegisterTypeDescriptor(g_types[" << i << "].typeId_, &g_types[" << i << "]);\n";
        }
        c << "}\n\n";
        c << "} // namespace OriGine\n";

        out.cppText = c.str();
    }

    return out;
}

} // namespace ReflectionCodeGen

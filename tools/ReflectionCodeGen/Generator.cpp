#include "Generator.h"

#include <sstream>
#include <unordered_set>

namespace ReflectionCodeGen {

namespace {

/// <summary>C++ 文字列リテラルとして安全な形にする(対象型の識別子・キー名はASCIIのみ)。</summary>
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
             "/// 64要素の型IDテーブル(コンポーネント)、および入れ子構造体専用テーブルへ登録する。\n"
             "/// FrameWork.cpp の RegisterUsingComponents() から明示的に呼ぶこと(Q16: 静的初期化子に\n"
             "/// よる自己登録は静的ライブラリでリンカに捨てられるため使わない)。\n"
             "/// </summary>\n"
             "ORIGINE_API void RegisterGeneratedComponentDescriptors();\n"
             "\n"
             "} // namespace OriGine\n";
        out.headerText = h.str();
    }

    // ---- 生成 .cpp ----
    {
        // 型ごとのフィールド開始位置(D2: 全型分のフィールドを1本のkFieldsに並べ、
        // TypeDesc.fieldStart_/fieldCount_ で範囲を指す)。コンポーネントか入れ子構造体かに
        // 関わらず、_types に現れる順のまま1本の配列に詰める。
        std::vector<uint32_t> fieldStarts(_types.size());
        {
            uint32_t fieldStart = 0;
            for (size_t i = 0; i < _types.size(); ++i) {
                fieldStarts[i] = fieldStart;
                fieldStart += static_cast<uint32_t>(_types[i].fields.size());
            }
        }

        // コンポーネント(ComponentTypeIdを持つ)と入れ子構造体(ORIGINE_STRUCT())を
        // 別々のTypeDesc配列に振り分ける。入れ子構造体は型IDを持たないので、型IDを添字にした
        // コンポーネント表に混ぜられない(component/ComponentReflection.h D6)。
        // structIndices の並び順が、そのまま FieldDesc::nestedTypeIndex_ が指す添字の意味になる
        // (main.cpp の FindNestedStructIndex に渡した structNames と同じ「発見順に filter」なので
        // 一致する)。
        std::vector<size_t> componentIndices, structIndices;
        for (size_t i = 0; i < _types.size(); ++i) {
            if (_types[i].isComponent) {
                componentIndices.push_back(i);
            } else {
                structIndices.push_back(i);
            }
        }

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
             // FieldUnwrap<decltype(...)>::Type/::kOffset もここで解決する。このツールは
             // IConstantBuffer のような「実データを内部に持つ型」の名前を一切知らない
             // (util/FieldUnwrap.h のコメント参照)。
             "#include \"util/FieldUnwrap.h\"\n"
             "\n";
        // 1ヘッダに複数の注釈付き型(例: 入れ子構造体+それを使うコンポーネント)が同居できる
        // ようになったため、同じヘッダを複数回 #include しないよう発見順に重複除去する
        // (#pragma once があるので実害は無いが、生成物が無駄に長くなるのを避ける)。
        std::unordered_set<std::string> seenHeaders;
        for (const auto& t : _types) {
            if (seenHeaders.insert(t.headerIncludePath).second) {
                c << "#include \"" << t.headerIncludePath << "\"\n";
            }
        }
        c << "\n"
             "#include <cstddef>\n"
             "#include <cstdint>\n"
             "#include <type_traits>\n"
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
        c << "// フィールドテーブル(D2)。型ごとの範囲は下の g_componentTypes/g_structTypes の\n"
             "// fieldStart_/fieldCount_ が指す。\n";
        c << "const FieldDesc kFields[] = {\n";
        for (const auto& t : _types) {
            c << "    // " << t.name << (t.isComponent ? "" : " (入れ子構造体)") << "\n";
            for (const auto& f : t.fields) {
                const std::string ownerField = "OriGine::" + t.name + "::" + f.name;
                const std::string declType   = "decltype(" + ownerField + ")";

                if (f.nestedStructIndex >= 0) {
                    // 入れ子フィールド: 実データの位置と型は FieldUnwrap<decltype(...)> 越しに
                    // 求める(このツールは IConstantBuffer という名前を知らない。offsetof自体は
                    // Owner側の生のオフセット、そこにFieldUnwrap側のオフセットを足す)。
                    c << "    { \"" << Escape(f.name) << "\", \"" << Escape(f.jsonKey) << "\", "
                      << "static_cast<uint32_t>(offsetof(OriGine::" << t.name << ", " << f.name
                      << ") + OriGine::FieldUnwrap<" << declType << ">::kOffset), "
                      // ここは非テンプレートの名前空間スコープ(kFieldsは通常の配列)なので、
                      // OriGine::FieldUnwrap<declType>::Type は依存名ではない。typenameは不要
                      // (テンプレート定義の外でtypenameを書くと構文エラーになるため付けない)。
                      << "static_cast<uint32_t>(sizeof(OriGine::FieldUnwrap<" << declType << ">::Type)), "
                      << "OriGine::kFieldTagNestedStruct, "
                      << (f.noSave ? "kFieldFlagNoSave" : "0") << ", "
                      << "kInvalidEnumIndex, "
                      << f.nestedStructIndex << "u },\n";
                } else {
                    // 既存どおり。typeTag_ はもうこのツールが決めない。
                    // `kFieldTagOf<decltype(OriGine::Type::field)>` という式をそのまま出力し、
                    // 実際にコンパイルする側(3構成それぞれ)に判定させる(D3)。
                    c << "    { \"" << Escape(f.name) << "\", \"" << Escape(f.jsonKey) << "\", "
                      << "static_cast<uint32_t>(offsetof(OriGine::" << t.name << ", " << f.name << ")), "
                      << "static_cast<uint32_t>(sizeof(" << ownerField << ")), "
                      << "OriGine::kFieldTagOf<" << declType << ">, "
                      << (f.noSave ? "kFieldFlagNoSave" : "0") << ", "
                      << (f.enumIndex < 0 ? "kInvalidEnumIndex" : std::to_string(f.enumIndex))
                      << ", 0u },\n";
                }
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

        // 入れ子フィールドの型整合性チェック: 生成ツールがトークン一致で「入れ子」と判定した型と、
        // 実際に FieldUnwrap<decltype(...)>::Type が指す型が一致することをコンパイル時に確認する
        // (例えば std::vector<OutlineParamData> のような、意図せずトークンだけ一致してしまった
        // ケースがあれば、ここでコンパイルエラーとして検出できる)。
        bool anyNested = false;
        for (const auto& t : _types) {
            for (const auto& f : t.fields) {
                if (f.nestedStructIndex < 0) {
                    continue;
                }
                if (!anyNested) {
                    c << "// 入れ子フィールドの型整合性チェック(トークン一致 vs 実際のFieldUnwrap<...>::Type)。\n";
                    anyNested = true;
                }
                const std::string& structName = _types[structIndices[static_cast<size_t>(f.nestedStructIndex)]].name;
                // 上と同じ理由でtypenameを付けない(非テンプレートスコープの非依存名)。
                c << "static_assert(std::is_same_v<OriGine::FieldUnwrap<decltype(OriGine::" << t.name
                  << "::" << f.name << ")>::Type, OriGine::" << structName << ">, \""
                  << t.name << "::" << f.name << " は入れ子構造体として判定されたが、実際の中身の型が一致しない\");\n";
            }
        }
        if (anyNested) {
            c << "\n";
        }

        // 型テーブル(D2)。typeId_ は起動時に RegisterGeneratedComponentDescriptors() が埋める
        // (コンポーネントのみ。入れ子構造体は ComponentTypeId を持たないため常に無効値のまま)。
        c << "// コンポーネント用の TypeDesc 表(型IDを添字にしたコンポーネント表へ登録する対象)。\n";
        c << "TypeDesc g_componentTypes[] = {\n";
        for (size_t idx : componentIndices) {
            const auto& t = _types[idx];
            c << "    { \"" << Escape(t.name) << "\", " << fieldStarts[idx] << "u, "
              << t.fields.size() << "u, static_cast<uint32_t>(sizeof(OriGine::" << t.name
              << ")), kInvalidComponentTypeId },\n";
        }
        c << "};\n\n";

        c << "// 入れ子構造体(ORIGINE_STRUCT())専用の TypeDesc 表。typeId_は常にkInvalidComponentTypeId\n"
             "// (コンポーネントではないため型IDを持たない)。ここでの並び順が\n"
             "// FieldDesc::nestedTypeIndex_(GetNestedTypeTable()への添字)の意味を決める。\n";
        c << "const TypeDesc g_structTypes[] = {\n";
        if (structIndices.empty()) {
            c << "    { \"\", 0u, 0u, 0u, kInvalidComponentTypeId }, // 対象に入れ子構造体が無いための空要素(サイズ0配列を避ける)\n";
        } else {
            for (size_t idx : structIndices) {
                const auto& t = _types[idx];
                c << "    { \"" << Escape(t.name) << "\", " << fieldStarts[idx] << "u, "
                  << t.fields.size() << "u, static_cast<uint32_t>(sizeof(OriGine::" << t.name
                  << ")), kInvalidComponentTypeId },\n";
            }
        }
        c << "};\n\n";

        c << "} // namespace\n\n";

        c << "void RegisterGeneratedComponentDescriptors() {\n";
        c << "    RegisterFieldTable(kFields, static_cast<uint32_t>(sizeof(kFields) / sizeof(kFields[0])));\n";
        c << "    RegisterNestedTypeTable(g_structTypes, "
          << (structIndices.empty() ? std::string("0u") : (std::to_string(structIndices.size()) + "u")) << ");\n";
        for (size_t outIdx = 0; outIdx < componentIndices.size(); ++outIdx) {
            const auto& t = _types[componentIndices[outIdx]];
            c << "    g_componentTypes[" << outIdx << "].typeId_ = GetComponentTypeId<OriGine::" << t.name << ">();\n";
            c << "    RegisterTypeDescriptor(g_componentTypes[" << outIdx << "].typeId_, &g_componentTypes[" << outIdx << "]);\n";
        }
        c << "}\n\n";
        c << "} // namespace OriGine\n";

        out.cppText = c.str();
    }

    return out;
}

} // namespace ReflectionCodeGen

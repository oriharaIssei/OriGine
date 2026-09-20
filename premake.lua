-- ==========================================================================
-- OriGine Engine premake helper
-- --------------------------------------------------------------------------
-- 2 つのモードで動作する:
--
--  (1) App 側 workspace premake から include される (通常用途)
--        App 側は `include "engine/premake.lua"` 後に以下を呼ぶ:
--          defineEngineProjects()
--          getEngineIncludeDirs()
--          getEngineLinks()
--
--  (2) Engine リポジトリ単独で直接実行される (standalone ビルド確認用)
--        $ cd OriGine (Engine repo root)
--        $ .\tools\premake5.exe vs2026
--        → _standalone/OriGine-Standalone.sln が生成され、Engine だけ
--          StaticLib としてビルドしてコンパイル確認できる。
--
-- ==========================================================================

-- 与えられた engineRoot ("engine" or "." or "") 下の相対パスを組み立てるヘルパ
local function p(engineRoot, sub)
    if engineRoot == nil or engineRoot == "" or engineRoot == "." then
        return sub
    else
        return engineRoot .. "/" .. sub
    end
end

-- --------------------------------------------------------------------------
-- Public API
-- --------------------------------------------------------------------------

function getEngineIncludeDirs(engineRoot)
    engineRoot = engineRoot or "engine"

    local base = (engineRoot == "." or engineRoot == "")
        and "$(SolutionDir)"
        or  "$(SolutionDir)" .. engineRoot .. "/"
    return {
        "$(SolutionDir)" .. (engineRoot == "." and "" or engineRoot),
        base .. "code",
        base .. "code/ECS",
        base .. "math",
        base .. "util",
        base .. "externals",
        base .. "externals/assimp/include",
    }
end

function getEngineLinks()
    return { "DirectXTex", "imgui" }
end

function defineEngineProjects(engineRoot)
    engineRoot = engineRoot or "engine"

    -- standalone(engine単体StaticLibビルド確認)では DLL 境界が存在しないため、
    -- OriGine を StaticLib のままにし ORIGINE_BUILD_DLL も定義しない(OriGineApi.h の3分岐目)。
    -- 通常(App workspace 経由)は SharedLib にし、ORIGINE_BUILD_DLL を定義してエクスポートする。
    local isStandalone = (engineRoot == "." or engineRoot == "")

    -- ----------------------------------------------------------------------
    -- OriGine (Engine static library)
    -- ----------------------------------------------------------------------
    project "OriGine"
        kind (isStandalone and "StaticLib" or "SharedLib")
        language "C++"
        location(engineRoot == "." and "." or engineRoot)
        targetdir "../generated/output/%{cfg.buildcfg}/"
        objdir "../generated/obj/%{cfg.buildcfg}/OriGine/"
        debugdir "%{wks.location}"

        files {
            p(engineRoot, "EngineInclude.h"),
            p(engineRoot, "code/**.h"),   p(engineRoot, "code/**.cpp"),
            p(engineRoot, "math/**.h"),   p(engineRoot, "math/**.cpp"),
            p(engineRoot, "util/**.h"),   p(engineRoot, "util/**.cpp"),
            p(engineRoot, "editor/**.h"), p(engineRoot, "editor/**.cpp"),
        }
        removefiles { p(engineRoot, "externals/**") }

        includedirs(getEngineIncludeDirs(engineRoot))

        dependson { "DirectXTex", "imgui", "ReflectionCodeGen" }
        links { "DirectXTex", "imgui" }

        defines { "_WINDOWS" }
        if not isStandalone then
            -- OriGine を DLL にする 4D の前提(docs/plans/phase-04.md D-1)。
            -- OriGineApi.h の ORIGINE_API はこのマクロの有無で dllexport/dllimport/空を切り替える。
            defines { "ORIGINE_BUILD_DLL" }
        end
        warnings "Extra"
        -- C4251(dllインターフェースのクラスがSTLメンバを持つ)/C4275(dllインターフェースでない
        -- 基底クラスをdllインターフェースの派生クラスが使う。例: ICollider→Collider<T>→各Collider)を抑止する(4D D-3)。
        -- 前提: OriGine.dll / ECS_TestGame.exe / ECS_TestEditor.exe は常に同じコンパイラ・
        -- 同じCRT(/MD,4A)/同じ構成(Debug/Develop/Release)で一緒にビルドされる。
        -- この前提が崩れる(片方だけ再ビルドする等)なら、この抑止は外して個別に対処すること。
        disablewarnings { "4251", "4275" }
        multiprocessorcompile "On"
        buildoptions { "/utf-8" }

        filter "configurations:Debug"
            -- ORIGINE_EDITOR_ENABLED: エディタUIの条件は `_DEBUG` から独立させる(4B B-2)。
            -- Debug/Develop 両方で定義し、Release では未定義のまま(ORIGINE_CALL_COUNTER_ENABLED と同じ命名規則)。
            defines { "DEBUG", "_DEBUG", "ORIGINE_EDITOR_ENABLED" }
            symbols "On"
            runtime "Debug"
            libdirs { p(engineRoot, "externals/assimp/lib/Debug") }
            links { "assimp-vc143-mdd" }
            -- staticruntime "Off"(4A): OriGine を DLL にする 4D の前提として CRT を動的リンクへ切り替える。
            -- 静的 CRT のままモジュール境界(DLL)を跨いで new/delete すると、CRT ヒープが
            -- モジュールごとに分かれて壊れる。4A ではまだ DLL を作らないが、CRT の差と境界の差を
            -- 混ぜて計測しないために先に切り替えておく(docs/plans/phase-04.md Q2)。
            -- 同一プロセスに載る全プロジェクト(OriGine / ReflectionCodeGen / DirectXTex / imgui / App)
            -- を同じ CRT リンク方式に揃えないと LNK2038(RuntimeLibrary mismatch)で落ちるため、
            -- assimp 非依存のプロジェクトも含め全プロジェクト・全構成で切り替える。
            staticruntime "Off"

        -- AssetCooker prebuild は submodule モードのみ有効
        -- (standalone では App 側 resource が無いのでスキップ)
        filter { "configurations:Debug", "system:windows" }
            if engineRoot ~= "." and engineRoot ~= "" and not os.getenv("CI") then
                prebuildcommands {
                    'pushd "%{wks.location}\\' .. engineRoot .. '\\externals\\assetCooker" && AssetCooker.exe -no_ui && popd'
                }
            end

        -- 型ディスクリプタ生成(Phase 3C, C-4)。ReflectionCodeGen は毎回のプリビルドで
        -- 実行されるが、内容ハッシュが変わらない限り生成物を書き換えない(=タイムスタンプも
        -- 変わらない)ので、"何も変えないビルド" でも OriGine の再コンパイルは起きない
        -- (6章の罠12番)。生成物は code/ECS/component/generated/ にコミットしてあるので、
        -- このコマンドが(standalone モードのように)走らなくてもビルド自体は通る。
        -- AssetCooker と同じ理由で standalone モードはパスの前提が異なるためスキップする。
        filter { "system:windows" }
            if engineRoot ~= "." and engineRoot ~= "" then
                prebuildcommands {
                    '"%{wks.location}\\..\\generated\\output\\%{cfg.buildcfg}\\ReflectionCodeGen.exe"'
                        .. ' --code-root "%{wks.location}\\' .. engineRoot .. '\\code\\ECS"'
                        .. ' --manifest "%{wks.location}\\' .. engineRoot .. '\\tools\\ReflectionCodeGen\\targets.txt"'
                        .. ' --out "%{wks.location}\\' .. engineRoot .. '\\code\\ECS\\component\\generated"'
                }
            end

        filter "configurations:Develop"
            -- ORIGINE_EDITOR_ENABLED: Develop でもエディタを有効にする(Editor.exe を Develop で動かすため)。
            defines { "DEVELOP", "_DEVELOP", "ORIGINE_EDITOR_ENABLED" }
            symbols "On"
            -- 計測用の構成。エンジン側が /Od だと計測対象そのものが実際の
            -- 実行コードと違うものになり、Release で消えるはずの呼び出しコストが
            -- 計測結果に混ざって最適化の判断を誤らせるため、アプリ側と同じく最適化を効かせる。
            optimize "Speed"
            runtime "Release"
            libdirs { p(engineRoot, "externals/assimp/lib/Release") }
            links { "assimp-vc143-md" }
            staticruntime "Off" -- 4A。理由は Debug 側のコメント参照

        filter "configurations:Release"
            defines { "NDEBUG", "_RELEASE", "RELEASE" }
            optimize "Full"
            runtime "Release"
            libdirs { p(engineRoot, "externals/assimp/lib/Release") }
            links { "assimp-vc143-md" }
            staticruntime "Off" -- 4A。理由は Debug 側のコメント参照

        filter "system:windows"
            cppdialect "C++20"
            systemversion "latest"
            postbuildcommands {
                "copy \"$(WindowsSdkDir)bin\\$(TargetPlatformVersion)\\x64\\dxcompiler.dll\" \"$(TargetDir)dxcompiler.dll\"",
                "copy \"$(WindowsSdkDir)bin\\$(TargetPlatformVersion)\\x64\\dxil.dll\" \"$(TargetDir)dxil.dll\""
            }

    -- ----------------------------------------------------------------------
    -- ReflectionCodeGen (型ディスクリプタ生成ツール。Phase 3C)
    -- エンジンのヘッダに依存しない(単体でビルドできる)単独の ConsoleApp。
    -- OriGine のプリビルドから呼ばれ、targets.txt に列挙したヘッダを読んで
    -- code/ECS/component/generated/ 以下の .h/.cpp を書く。
    -- ----------------------------------------------------------------------
    project "ReflectionCodeGen"
        kind "ConsoleApp"
        language "C++"
        location(p(engineRoot, "tools/ReflectionCodeGen"))
        targetdir "../generated/output/%{cfg.buildcfg}/"
        objdir "../generated/obj/%{cfg.buildcfg}/ReflectionCodeGen/"

        files {
            p(engineRoot, "tools/ReflectionCodeGen/**.h"),
            p(engineRoot, "tools/ReflectionCodeGen/**.cpp"),
        }
        includedirs { p(engineRoot, "tools/ReflectionCodeGen") }

        warnings "Extra"
        multiprocessorcompile "On"
        buildoptions { "/utf-8" }

        filter "system:windows"
            cppdialect "C++20"
            systemversion "latest"

        -- staticruntime "Off"(4A): ReflectionCodeGen は OriGine とプロセスを共有しない単独ツールだが、
        -- 「全プロジェクト・全構成で揃える」という決定に従い、ここも静的 CRT をやめておく
        -- (docs/plans/phase-04.md 4A-2)。
        filter "configurations:Debug"
            symbols "On"
            runtime "Debug"
            staticruntime "Off"
        filter "configurations:Develop"
            symbols "On"
            optimize "Speed"
            runtime "Release"
            staticruntime "Off"
        filter "configurations:Release"
            optimize "Full"
            runtime "Release"
            staticruntime "Off"

    -- ----------------------------------------------------------------------
    -- DirectXTex
    -- ----------------------------------------------------------------------
    project "DirectXTex"
        kind "StaticLib"
        language "C++"
        location(p(engineRoot, "externals/DirectXTex/"))
        targetdir "../generated/output/%{cfg.buildcfg}/"
        objdir "../generated/obj/%{cfg.buildcfg}/DirectXTex/"
        targetname "DirectXTex"

        files {
            p(engineRoot, "externals/DirectXTex/**.h"),
            p(engineRoot, "externals/DirectXTex/**.cpp"),
        }
        includedirs { "$(ProjectDir)", "$(ProjectDir)Shaders/Compiled" }
        multiprocessorcompile "On"

        filter "system:windows"
            cppdialect "C++20"
            systemversion "latest"

        -- staticruntime "Off"(4A): OriGine に静的リンクされるので、OriGine と CRT リンク方式が
        -- 食い違うと LNK2038(RuntimeLibrary mismatch)になる。理由の詳細は OriGine 側のコメント参照。
        filter "configurations:Debug"
            runtime "Debug"
            symbols "On"
            staticruntime "Off"
        filter "configurations:Develop"
            runtime "Release"
            symbols "On"
            optimize "Speed"
            staticruntime "Off"
        filter "configurations:Release"
            runtime "Release"
            optimize "Full"
            staticruntime "Off"

    -- ----------------------------------------------------------------------
    -- imgui
    -- ----------------------------------------------------------------------
    project "imgui"
        kind "StaticLib"
        language "C++"
        location(p(engineRoot, "externals/imgui/"))
        targetdir "../generated/output/%{cfg.buildcfg}/"
        objdir "../generated/obj/%{cfg.buildcfg}/imgui/"

        includedirs { "$(ProjectDir)", "$(ProjectDir)/imgui" }
        files {
            p(engineRoot, "externals/imgui/**.h"),
            p(engineRoot, "externals/imgui/**.cpp"),
        }
        multiprocessorcompile "On"

        filter "system:windows"
            cppdialect "C++20"
            systemversion "latest"
        -- staticruntime "Off"(4A): OriGine に静的リンクされるので DirectXTex と同じ理由で揃える。
        -- runtime も明示する(4D で発見: この指定が無いと premake は既定で Release 版の動的CRT
        -- (MSVCRT)を選ぶらしく、Debug構成でも imgui.lib だけ MSVCRTD ではなく MSVCRT を
        -- defaultlib指定してしまっていた。OriGine が静的ライブラリのままだった間はOriGine.lib
        -- 自体がリンクされることが無く/WHOLEARCHIVEもリンク時のdefaultlibチェックも走らなかった
        -- ため気づかれなかったが、OriGine が SharedLib になり実際にリンクが走るようになったことで
        -- LNK4098(defaultlib競合)警告として顕在化した。実害(即クラッシュ等)は無かったが、
        -- 4Aの目的(全プロジェクト/全構成でCRTリンク方式を揃える)そのものに反するため直す。
        filter "configurations:Debug"
            runtime "Debug"
            staticruntime "Off"
        filter "configurations:Develop"
            runtime "Release"
            optimize "Speed"
            staticruntime "Off"
        filter "configurations:Release"
            runtime "Release"
            staticruntime "Off"
end

-- ==========================================================================
-- Standalone モード
-- --------------------------------------------------------------------------
-- premake5 がこの premake.lua を直接 --file= で読み込んだ場合のみ発動。
-- App 側 premake から include された場合は _MAIN_SCRIPT が App 側を指すので
-- この分岐には入らず、上記の関数定義だけが export される。
-- ==========================================================================
if _MAIN_SCRIPT == _SCRIPT and _ACTION ~= nil then
    workspace "OriGine-Standalone"
        location "_standalone"
        architecture "x86_64"
        configurations { "Debug", "Develop", "Release" }
        startproject "OriGine"

    -- Engine リポジトリ自身をルートとして全 project を定義
    defineEngineProjects(".")
end

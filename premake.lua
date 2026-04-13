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

    -- ----------------------------------------------------------------------
    -- OriGine (Engine static library)
    -- ----------------------------------------------------------------------
    project "OriGine"
        kind "StaticLib"
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

        dependson { "DirectXTex", "imgui" }
        links { "DirectXTex", "imgui" }

        defines { "_WINDOWS" }
        warnings "Extra"
        multiprocessorcompile "On"
        buildoptions { "/utf-8" }

        filter "configurations:Debug"
            defines { "DEBUG", "_DEBUG" }
            symbols "On"
            runtime "Debug"
            libdirs { p(engineRoot, "externals/assimp/lib/Debug") }
            links { "assimp-vc143-mtd" }
            staticruntime "On"

        -- AssetCooker prebuild は submodule モードのみ有効
        -- (standalone では App 側 resource が無いのでスキップ)
        filter { "configurations:Debug", "system:windows" }
            if engineRoot ~= "." and engineRoot ~= "" and not os.getenv("CI") then
                prebuildcommands {
                    'pushd "%{wks.location}\\' .. engineRoot .. '\\externals\\assetCooker" && AssetCooker.exe -no_ui && popd'
                }
            end

        filter "configurations:Develop"
            defines { "DEVELOP", "_DEVELOP" }
            symbols "On"
            runtime "Release"
            libdirs { p(engineRoot, "externals/assimp/lib/Release") }
            links { "assimp-vc143-mt" }
            staticruntime "On"

        filter "configurations:Release"
            defines { "NDEBUG", "_RELEASE", "RELEASE" }
            optimize "Full"
            runtime "Release"
            libdirs { p(engineRoot, "externals/assimp/lib/Release") }
            links { "assimp-vc143-mt" }
            staticruntime "On"

        filter "system:windows"
            cppdialect "C++20"
            systemversion "latest"
            postbuildcommands {
                "copy \"$(WindowsSdkDir)bin\\$(TargetPlatformVersion)\\x64\\dxcompiler.dll\" \"$(TargetDir)dxcompiler.dll\"",
                "copy \"$(WindowsSdkDir)bin\\$(TargetPlatformVersion)\\x64\\dxil.dll\" \"$(TargetDir)dxil.dll\""
            }

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

        filter "configurations:Debug"
            runtime "Debug"
            symbols "On"
            staticruntime "On"
        filter "configurations:Develop"
            runtime "Release"
            symbols "On"
            staticruntime "On"
        filter "configurations:Release"
            runtime "Release"
            optimize "Full"
            staticruntime "On"

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
        filter "configurations:Debug"
            staticruntime "On"
        filter "configurations:Develop"
            staticruntime "On"
        filter "configurations:Release"
            staticruntime "On"
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

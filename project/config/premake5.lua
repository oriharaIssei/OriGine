-- 先頭で作業ディレクトリをprojectに変更
os.chdir(_SCRIPT_DIR .. "/..")

workspace "OriGine"

    architecture "x86_64"
    configurations { "Debug", "Release" }

    startproject "OriGine"

project "OriGine"
    kind "WindowedApp"
    language "C++"
    targetdir "../generated/output/%{cfg.buildcfg}/"
    objdir "../generated/obj/%{cfg.buildcfg}/OriGine/"

    files { "**.h", "**.cpp"}
    removefiles  { "externals/**", "application/**"}

    includedirs {
        "$(SolutionDir)math",
        "$(SolutionDir)lib",
        "$(SolutionDir)engine/code/ECS",
        "$(SolutionDir)engine/code",
        "$(SolutionDir)engine",
        "$(SolutionDir).",
        "$(SolutionDir)externals",
        "$(SolutionDir)externals/assimp/include"
    }

    dependson {
        "DirectXTex",
        "imgui",
        "OriGineApp"
        }

    links {
        "DirectXTex",
        "imgui",
        "OriGineApp"
    }

    defines {
        "_WINDOWS"
    }

    warnings "Extra"
    buildoptions { "/WX", "/utf-8" }

    filter "configurations:Debug"
        defines { "DEBUG", "_DEBUG" }
        symbols "On"
        runtime "Debug"
        libdirs { "externals/assimp/lib/Debug" }
        links { "assimp-vc143-mtd" }
        staticruntime "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "Full"
        runtime "Release"
        libdirs { "externals/assimp/lib/Release" }
        links { "assimp-vc143-mt" }
        staticruntime "On"

    filter "system:windows"
        cppdialect "C++20"
        systemversion "latest"
        postbuildcommands {
            "copy \"$(WindowsSdkDir)bin\\$(TargetPlatformVersion)\\x64\\dxcompiler.dll\" \"$(TargetDir)dxcompiler.dll\"",
            "copy \"$(WindowsSdkDir)bin\\$(TargetPlatformVersion)\\x64\\dxil.dll\" \"$(TargetDir)dxil.dll\""
        }

project "DirectXTex"
    kind "StaticLib"
    language "C++"
    location "externals/DirectXTex/"
    targetdir "../generated/output/%{cfg.buildcfg}/"
    objdir "../generated/obj/%{cfg.buildcfg}/DirectXTex/"
    targetname "DirectXTex"
    files { "externals/DirectXTex/**.h", "externals/DirectXTex/**.cpp" }
    includedirs { "$(ProjectDir)","$(ProjectDir)Shaders/Compiled" }
    filter "system:windows"
        cppdialect "C++20"
        systemversion "latest"
    filter "configurations:Debug"
         runtime "Debug"       -- Debug ランタイム (MTd) を使用
         symbols "On"
         staticruntime "On"
    filter "configurations:Release"
         runtime "Release"     -- Release ランタイム (MT) を使用
         optimize "Full"
         staticruntime "On"

project "imgui"
    kind "StaticLib"
    language "C++"
    location "externals/imgui/"
    targetdir "../generated/output/%{cfg.buildcfg}/"
    objdir "../generated/obj/%{cfg.buildcfg}/imgui/"

    files { "externals/imgui/**.h", "externals/imgui/**.cpp" }

    filter "system:windows"
        cppdialect "C++20"
        systemversion "latest"

    filter "configurations:Debug"
        staticruntime "On"

    filter "configurations:Release"
        staticruntime "On"

project "OriGineApp"
    kind "StaticLib"
    language "C++"
    location "application"
    targetdir "../generated/output/%{cfg.buildcfg}/"
    objdir "../generated/obj/%{cfg.buildcfg}/OriGineApp/"
    files { "application/**.h", "application/**.cpp" }

    includedirs {
        "$(ProjectDir)code",
        "$(ProjectDir)",
        "$(SolutionDir)math",
        "$(SolutionDir)lib",
        "$(SolutionDir)engine/code/ECS",
        "$(SolutionDir)engine/code",
        "$(SolutionDir)engine",
        "$(SolutionDir).",
        "$(SolutionDir)externals",
        "$(SolutionDir)externals/assimp/include"
    }
    dependson { "DirectXTex", "imgui" }
    links {
        "DirectXTex",
        "imgui"
    }
    warnings "Extra"
    buildoptions { "/WX", "/utf-8","/bigobj" }

    filter "configurations:Debug"
        defines { "DEBUG", "_DEBUG" }
        symbols "On"
        runtime "Debug"
        libdirs { "externals/assimp/lib/Debug" }
        links { "assimp-vc143-mtd" }
        staticruntime "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "Full"
        runtime "Release"
        libdirs { "externals/assimp/lib/Release" }
        links { "assimp-vc143-mt" }
        staticruntime "On"

    filter "system:windows"
        cppdialect "C++20"
        systemversion "latest"

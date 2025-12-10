-- 先頭で作業ディレクトリをprojectに変更
os.chdir(_SCRIPT_DIR .. "/..")

workspace "OriGine"

    architecture "x86_64"
    configurations { "Debug", "Develop", "Release" }

    startproject "OriGineApp"

project "OriGine"
    kind "StaticLib"
    language "C++"
    targetdir "../generated/output/%{cfg.buildcfg}/"
    objdir "../generated/obj/%{cfg.buildcfg}/OriGine/"
    -- デバッグ時の作業ディレクトリを指定
    debugdir "%{wks.location}"
    files { "**.h", "**.cpp"}
    removefiles  { "externals/**", "application/**"}

    -- Clang-Tidy 有効化
    clangtidy "On"

    includedirs {
        "$(SolutionDir)math",
        "$(SolutionDir)util",
        "$(SolutionDir)engine/code/ECS",
        "$(SolutionDir)engine/code",
        "$(SolutionDir)engine",
        "$(SolutionDir).",
        "$(SolutionDir)externals",
        "$(SolutionDir)externals/assimp/include"
    }

    dependson {
        "DirectXTex",
        "imgui"
        }

    links {
        "DirectXTex",
        "imgui"
    }

    defines {
        "_WINDOWS"
    }

    warnings "Extra"
    buildoptions { "/utf-8", "/MP" }

    filter "configurations:Debug"
        defines { "DEBUG", "_DEBUG" }
        symbols "On"
        runtime "Debug"
        libdirs { "externals/assimp/lib/Debug" }
        links { "assimp-vc143-mtd" }
        staticruntime "On"
    filter "configurations:Develop"
        defines { "DEVELOP", "_DEVELOP" }
        symbols "On"
        runtime "Release" -- 開発用のリリースビルド
        libdirs { "externals/assimp/lib/Release" }
        links { "assimp-vc143-mt" }
        staticruntime "On"
    filter "configurations:Release"
        defines { "NDEBUG","_RELEASE","RELEASE" }
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
     filter "configurations:Develop"
        runtime "Release" -- 開発用のリリースビルド
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

    includedirs {
        "$(ProjectDir)",
        "$(ProjectDir)/imgui"
    }

    files { "externals/imgui/**.h", "externals/imgui/**.cpp" }

    filter "system:windows"
        cppdialect "C++20"
        systemversion "latest"
    filter "configurations:Debug"
        staticruntime "On"
    filter "configurations:Develop"
        staticruntime "On"
    filter "configurations:Release"
        staticruntime "On"

project "OriGineApp"
    kind "WindowedApp"
    language "C++"
    location "application"
    targetdir "../generated/output/%{cfg.buildcfg}/"
    objdir "../generated/obj/%{cfg.buildcfg}/OriGineApp/"
    -- デバッグ時の作業ディレクトリを指定
    debugdir "%{wks.location}"
    files { "application/**.h", "application/**.cpp" }

    -- Clang-Tidy 有効化
    clangtidy "On"

    includedirs {
        "$(ProjectDir)code",
        "$(ProjectDir)",
        "$(SolutionDir)math",
        "$(SolutionDir)util",
        "$(SolutionDir)engine/code/ECS",
        "$(SolutionDir)engine/code",
        "$(SolutionDir)engine",
        "$(SolutionDir).",
        "$(SolutionDir)externals",
        "$(SolutionDir)externals/assimp/include"
    }
    dependson { "DirectXTex", "imgui" }
    links {
        "OriGine",
        "DirectXTex",
        "imgui"
    }
    warnings "Extra"
    buildoptions { "/utf-8","/bigobj", "/MP" }

    filter "configurations:Debug"
        defines { "DEBUG", "_DEBUG" }
        symbols "On"
        runtime "Debug"
        libdirs { "externals/assimp/lib/Debug" }
        links { "assimp-vc143-mtd" }
        staticruntime "On"

    filter "configurations:Develop"
        defines { "DEVELOP", "_DEVELOP" }
        symbols "On"
        runtime "Release" -- 開発用のリリースビルド
        libdirs { "externals/assimp/lib/Release" }
        links { "assimp-vc143-mt" }
        staticruntime "On"

    filter "configurations:Release"
        defines { "NDEBUG", "_RELEASE", "RELEASE" }
        optimize "Full"
        runtime "Release"
        libdirs { "externals/assimp/lib/Release" }
        links { "assimp-vc143-mt" }
        staticruntime "On"

    filter "system:windows"
        cppdialect "C++20"
        systemversion "latest"

workspace "OriGine"
    architecture "x86_64"
    configurations { "Debug", "Release" }

    startproject "OriGine"

project "OriGine"
    kind "WindowedApp"
    language "C++"
    targetdir "../generated/outputs/%{cfg.buildcfg}/"

    files { "OriGine/**.h", "OriGine/**.cpp" }

    includedirs {
        "OriGine/code/math",
        "OriGine/code/lib",
        "OriGine/code/gameScene",
        "OriGine/code/engine",
        "OriGine/code",
        "project/externals/assimp/include",
        "project/externals/imgui_nodeEditor"
    }

    links {
        "DirectXTex",
        "imgui"
    }

    defines {
        "_WINDOWS"
    }

    filter "configurations:Debug"
        defines { "DEBUG", "_DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "Full"

    filter "system:windows"
        cppdialect "C++20"
        systemversion "latest"
        postbuildcommands {
            "{COPY} $(WindowsSdkDir)bin/$(TargetPlatformVersion)/x64/dxcompiler.dll %{cfg.targetdir}/dxcompiler.dll",
            "{COPY} $(WindowsSdkDir)bin/$(TargetPlatformVersion)/x64/dxil.dll %{cfg.targetdir}/dxil.dll"
        }

project "DirectXTex"
    kind "StaticLib"
    language "C++"
    location "project/externals/DirectXTex/"
    targetdir "../generated/outputs/DirectXTex/%{cfg.buildcfg}/"

    files { "project/externals/DirectXTex/**.h", "project/externals/DirectXTex/**.cpp" }

    filter "system:windows"
        cppdialect "C++20"
        systemversion "latest"

project "imgui"
    kind "StaticLib"
    language "C++"
    location "project/externals/imgui/"
    targetdir "../generated/outputs/imgui/%{cfg.buildcfg}/"

    files { "project/externals/imgui/**.h", "project/externals/imgui/**.cpp" }

    filter "system:windows"
        cppdialect "C++20"
        systemversion "latest"

workspace "OriGine"
    configurations { "Debug", "Release", "Profile" }
    platforms { "x64", "Win32" }  -- 追加
    location "project"
    startproject "OriGine"

project "OriGine"
    -- プロジェクトの種類を指定 （WindowedApp, ConsoleApp, StaticLib, SharedLib）
    kind "WindowedApp"
    language "C++"
    -- C++20 を指定
    cppdialect "C++20"
    -- プロジェクトのフォルダーを指定
    location "./project"
    -- プロジェクトの出力先を指定
    targetdir "../OriGine/generated/outputs/%{cfg.buildcfg}"
    -- 中間ファイルの出力先を指定
    objdir "../OriGine/generated/objects/%{cfg.buildcfg}"
    -- ビルド後イベントの追加
   postbuildcommands {
        '{COPY} "%{os.getenv("WindowsSdkDir")}bin\\%{os.getenv("TargetPlatformVersion")}\\x64\\dxcompiler.dll" "%{cfg.targetdir}\\dxcompiler.dll"',
        '{COPY} "%{os.getenv("WindowsSdkDir")}bin\\%{os.getenv("TargetPlatformVersion")}\\x64\\dxil.dll" "%{cfg.targetdir}\\dxil.dll"'
    }

    files { "project/code/**.h", "project/code/**.cpp"}
    -- 除外するファイルやフォルダーを指定
    excludes { "project/externals/imgui/**",
               "project/externals/DirectXTex/**",
               "project/externals/assimp/**",
               "project/resource"
            }

    -- imgui と DirectXTex を参照
    links { "imgui", "DirectXTex" }

    -- インクルードディレクトリの追加
    includedirs { "project/code","project/code/engine","project/code/application","project/code/lib","project/code/math", -- project Include
                  "project/externals","project/externals/imgui", "project/externals/DirectXTex","project/externals/assimp/include" } -- externals Include

    buildoptions { "/utf-8" }

    filter "platforms:x64"
        architecture "x64"

    filter "platforms:Win32"
        architecture "x86"

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
        staticruntime "on" -- runtime "Debug" と 組み合わせることで /MTd になる
        runtime "Debug"
        links {"assimp-vc143-mtd.lib" }
        libdirs {"project/externals/assimp/lib/Debug" }

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
        staticruntime "off" -- runtime "Release" と 組み合わせることで /MT になる
        runtime "Release"
        links {"assimp-vc143-mt.lib" }
        libdirs {"project/externals/assimp/lib/Release" }

    filter "configurations:Profile"
        defines { "NDEBUG" }
        optimize "On"
        staticruntime "off"
        runtime "Release"
        links {"assimp-vc143-mt.lib" }
        libdirs { "project/externals/assimp/lib/Release" }

externalproject  "imgui"
    kind "StaticLib"
    language "C++"
    location "project/externals/imgui"

    targetdir "../OriGine/generated/outputs/%{cfg.buildcfg}"
    -- 中間ファイルの出力先を指定
    objdir "../OriGine/generated/objects/%{cfg.buildcfg}"

    files { "project/externals/imgui/**.h", "project/externals/imgui/**.cpp" }

    includedirs { "project/externals/imgui" }

    -- C++20 を指定
    cppdialect "C++20"

    buildoptions { "/utf-8" }

    filter "platforms:x64"
        architecture "x64"

    filter "platforms:Win32"
        architecture "x86"

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
        staticruntime "on"
        runtime "Debug"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
        staticruntime "off"
        runtime "Release"

externalproject  "DirectXTex"
    kind "StaticLib"
    language "C++"
    location "project/externals/DirectXTex"
    targetdir "../OriGine/generated/outputs/%{cfg.buildcfg}"
    -- 中間ファイルの出力先を指定
    objdir "../OriGine/generated/objects/%{cfg.buildcfg}"

    files { "project/externals/DirectXTex/**.h", "project/externals/DirectXTex/**.cpp" }

    includedirs { "project/externals/DirectXTex/Shaders", "project/externals/DirectXTex/Shaders/Compiled" }

    -- C++20 を指定
    cppdialect "C++20"

    -- Desktop_2022_Win10.vcxproj の設定に合わせる
    systemversion "latest"
    defines { "UNICODE", "_UNICODE", "WIN32", "_WIN32" }

    buildoptions { "/utf-8" }

    filter "platforms:x64"
        architecture "x64"

    filter "platforms:Win32"
        architecture "x86"

    filter "configurations:Debug"
        defines { "DEBUG", "_DEBUG" }
        symbols "On"
        staticruntime "on"
        runtime "Debug"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
        staticruntime "off"
        runtime "Release"

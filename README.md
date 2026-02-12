# 自作エンジン OriGine

[![DebugBuild](https://github.com/oriharaIssei/OriGine/actions/workflows/DebugBuild.yml/badge.svg)](https://github.com/oriharaIssei/OriGine/actions/workflows/DebugBuild.yml)

[![ReleaseBuild](https://github.com/oriharaIssei/OriGine/actions/workflows/ReleaseBuild.yml/badge.svg?branch=main)](https://github.com/oriharaIssei/OriGine/actions/workflows/ReleaseBuild.yml)

## ビルド方法

### 必要環境
* Windows 10/11
* Visual Studio 2026
* PowerShell
### プロジェクト生成
本プロジェクトは Premake5 を使用してプロジェクトファイルを生成しています。

### 生成方法
```powershell
.\premake5.ps1
```

### 任意ターゲット指定
引数にターゲットを指定することでターゲットを変更することができます。
```powershell
.\premake5.ps1 vs2022
.\premake5.ps1 gmake2
```
デフォルトでは、 ”**vs2026**”が指定されています。

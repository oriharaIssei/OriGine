# OriGine

**O**rihara-En**gine**. C++20 / DirectX12 ベースの自作ゲームエンジン。

---

## 使い方

本リポジトリは **Engine 単体** として公開されています。アプリケーションを作る場合は
[OriGine-AppTemplate](https://github.com/oriharaIssei/OriGine-AppTemplate) を
Template として利用し、本リポジトリを submodule として取り込んでください。

### 最短の始め方

```powershell
gh repo create MyGame --template oriharaIssei/OriGine-AppTemplate --public --clone
cd MyGame
.\setup.ps1 -AppName "MyGame" -EngineRepo "https://github.com/oriharaIssei/OriGine.git"
```

---

## ディレクトリ構成

```
OriGine/
├── premake.lua          # App 側から include される public API (関数 export)
├── EngineInclude.h      # マクロ分岐で Engine 各機能を集約する include ヘッダ
├── code/                # Engine 本体 (directX12, ECS, scene, input, ...)
├── math/                # Vector / Matrix / Quaternion / Spline / Easing 等
├── util/                # BinaryIO / FileSystem / GlobalVariables / MyGui ...
├── editor/              # エディタ基盤 (debugReplayer, sceneEditor, setting, ...)
├── externals/           # DirectXTex / imgui / assimp / nlohmann / spdlog / uuid
├── resource/            # Engine 付属リソース (標準 Shader / Model / Texture / fonts)
└── tools/               # premake5.exe, TextureConverter.exe
```

## 提供する premake API

App 側 workspace premake5.lua から `include "engine/premake.lua"` で取り込み、
下記 3 関数を呼び出す:

| 関数 | 役割 |
|---|---|
| `defineEngineProjects()` | `OriGine` (StaticLib) / `DirectXTex` / `imgui` project を定義 |
| `getEngineIncludeDirs()` | App project の `includedirs` に追加すべきパスを返す |
| `getEngineLinks()` | App project の `links` に追加すべき名前を返す |

## Engine 単独でのコンパイル確認

本リポジトリ単独で Engine のビルドが通るかを確認する場合:

```powershell
cd <OriGine repo root>
.\tools\premake5.exe vs2026
# → _standalone/OriGine-Standalone.sln が生成される
# Visual Studio で開いてビルド (Debug / Develop / Release)
# OriGine.lib が出力されれば Engine 単独でコンパイル可
```

Standalone モードでは:
- App 側 FrameWork や main は含まれない (StaticLib のみビルド)
- AssetCooker 前処理はスキップ (App 側 resource が存在しないため)
- 生成物は `_standalone/` 以下に出力され、`.gitignore` 対象

※ 未使用関数の静的ライブラリリンク検査はできないため、完全な動作確認は
OriGine-AppTemplate 側からの実アプリビルドで行ってください。

## 依存関係

- Windows + Visual Studio 2026 (or 互換)
- Windows SDK
- C++20

外部ライブラリは `externals/` にすべて同梱済み (prebuilt assimp lib 含む)。

---

## TODO

- [ ] `externals/assimp/lib/` (合計 184MB) を Git LFS へ移行検討
- [ ] Engine 単独でビルド確認する手段 (現状は App 側から使う前提)
- [ ] サンプルアプリの用意

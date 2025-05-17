param(
    [Parameter(Mandatory=$true)]
    [string]$ProjectName
)

# Releaseビルドの出力ディレクトリ
$buildDir = "generated/output/Release"

# パッケージ出力先
$packageDir = "$ProjectName"

# コピーするファイル拡張子
$extensions = @("*.exe", "*.lib","*.dll")

# exeファイルの元の名前（ビルド出力名に合わせて修正）
$originalExe = Get-ChildItem -Path $buildDir -Filter *.exe | Select-Object -First 1

# engine/resource, Application/resource のパス
$engineResourceSrc = "project\engine\resource"
$appResourceSrc    = "project\Application\resource"

# コピー先ディレクトリ
$engineResourceDst = Join-Path $packageDir "engine\resource"
$appResourceDst    = Join-Path $packageDir "Application\resource"

# パッケージディレクトリ作成
if (!(Test-Path $packageDir)) { New-Item -ItemType Directory -Path $packageDir }

# exeファイルをプロジェクト名にリネームしてコピー
if ($originalExe) {
    Copy-Item -Path $originalExe.FullName -Destination (Join-Path $packageDir "$ProjectName.exe") -Force
}

# dll, lib をパッケージ直下にコピー
foreach ($ext in $extensions) {
    Get-ChildItem -Path $buildDir -Filter $ext | Copy-Item -Destination $packageDir -Force
}

# engine/resource をコピー
if (Test-Path $engineResourceSrc) {
    Copy-Item -Path "$engineResourceSrc\*" -Destination $engineResourceDst -Recurse -Force
}

# Application/resource をコピー
if (Test-Path $appResourceSrc) {
    Copy-Item -Path "$appResourceSrc\*" -Destination $appResourceDst -Recurse -Force
}

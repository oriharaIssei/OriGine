param(
    [string]$TargetDir,
    [string]$FolderName
)

# === 共通関数 ===
function Ensure-Directory($path) {
    if (-not (Test-Path $path)) {
        New-Item -ItemType Directory -Path $path -Force | Out-Null
        Write-Host "Created $path" -ForegroundColor Green
    }
}

# === ターゲットフォルダー作成 ===
$TargetFolder = Join-Path $TargetDir $FolderName
if (-not $TargetFolder) {
    Write-Warning "ターゲットフォルダーのパスが正しくありません。"
    exit 1
}
Ensure-Directory $TargetFolder

# サブフォルダー作成
# === サブフォルダー作成 ===
$AppFolder = Join-Path $TargetFolder "application"
$EngineFolder = Join-Path $TargetFolder "engine"

# ★変更: resource フォルダのパスを定義
$AppResourceFolder = Join-Path $AppFolder "resource"
$EngineResourceFolder = Join-Path $EngineFolder "resource"

# ★変更: resource フォルダを作成 (親の application や engine も自動で作られます)
Ensure-Directory $AppResourceFolder
Ensure-Directory $EngineResourceFolder

# === ビルド ===
Write-Host "msbuildを開始しています..." -ForegroundColor Yellow

# 直接実行することで出力をリアルタイムで表示
& msbuild.exe "project/OriGine.sln" /p:Configuration=Release /nr:false

if ($LASTEXITCODE -ne 0) {
    Write-Warning "msbuildが失敗しました。終了コード: $LASTEXITCODE"
    exit 1
}
Write-Host "msbuildが完了しました。" -ForegroundColor Green

# === 実行ファイル・ライブラリコピー ===
$buildOutput = "generated/output/Release"
try {
    if (Test-Path $buildOutput) {
        Get-ChildItem $buildOutput -Recurse -Include *.exe, *.lib, *.dll -File -ErrorAction Stop |
        Copy-Item -Destination $TargetFolder -Force -ErrorAction Stop
        Write-Host "DLL/EXE/LIBファイルのコピーが完了しました。" -ForegroundColor Green
    }
    else {
        throw "$buildOutput が存在しません。ビルドに失敗している可能性があります。"
    }
}
catch {
    Write-Warning "ファイルコピー中にエラーが発生しました: $_"
    exit 1
}

# === Applicationリソースコピー ===
$sourceAppResource = "project/application/resource"
if (Test-Path $sourceAppResource) {
    Copy-Item "$sourceAppResource\*" -Destination $AppResourceFolder -Recurse -Force
    Write-Host "Applicationリソースを $AppResourceFolder にコピーしました。" -ForegroundColor Green
}
else {
    Write-Warning "Applicationリソースフォルダー ($sourceAppResource) が存在しません。"
}

# === engineリソースコピー ===
$sourceEngineResource = "project/engine/resource"
if (Test-Path $sourceEngineResource) {
    Copy-Item "$sourceEngineResource\*" -Destination $EngineResourceFolder -Recurse -Force
    Write-Host "engineリソースを $EngineResourceFolder にコピーしました。" -ForegroundColor Green
}
else {
    Write-Warning "engineリソースフォルダー ($sourceEngineResource) が存在しません。"
}


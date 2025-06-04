param(
    [string]$TargetDir,
    [string]$FolderName
)

# 0. ターゲットフォルダー作成
$TargetFolder = Join-Path $TargetDir $FolderName
if (-not $TargetFolder) {
    Write-Warning "ターゲットフォルダーのパスが正しくありません。"
    exit 1
}
New-Item -ItemType Directory -Path $TargetFolder -Force | Out-Null

# 1. premake5.lua 実行
& premake5 vs2022 --file=project/config/premake5.lua

# 2. msbuildでC++ビルド
msbuild project/OriGine.sln /p:Configuration=Release

# 3. .exeと.libをコピー
$buildOutput = "generated/output/Release"
if (Test-Path $buildOutput) {
    Get-ChildItem $buildOutput -Include *.exe,*.lib -File | ForEach-Object {
        Copy-Item $_.FullName -Destination $TargetFolder -Force
    }
} else {
    Write-Warning "$buildOutput が存在しません。ビルドに失敗している可能性があります。"
}

# 4. Applicationフォルダー作成
$AppFolder = Join-Path $TargetFolder "Application"
New-Item -ItemType Directory -Path $AppFolder -Force | Out-Null

# 5. engineフォルダー作成
$EngineFolder = Join-Path $TargetFolder "engine"
New-Item -ItemType Directory -Path $EngineFolder -Force | Out-Null

# 6. Applicationリソースコピー
Copy-Item -Path "project/Application/resource\*" -Destination $AppFolder -Recurse -Force

# 7. engineリソースコピー
Get-ChildItem -Path "project/engine/resource" -Recurse | Where-Object {
    -not ($_.PSIsContainer -and $_.Name -eq "logs")
} | ForEach-Object {
    $dest = $_.FullName.Replace((Resolve-Path "project/engine/resource").Path, $EngineFolder)
    if ($_.PSIsContainer) {
        if (-not (Test-Path $dest)) {
            New-Item -ItemType Directory -Path $dest -Force | Out-Null
        }
    } else {
        $destDir = Split-Path $dest
        if (-not (Test-Path $destDir)) {
            New-Item -ItemType Directory -Path $destDir -Force | Out-Null
        }
        Copy-Item $_.FullName -Destination $dest -Force
    }
}

# 8. zip化
$zipPath = "${TargetFolder}.zip"
if (Test-Path $TargetFolder) {
    Compress-Archive -Path "$TargetFolder\*" -DestinationPath $zipPath -Force
    Write-Host "パッケージ化が完了しました: $zipPath"
} else {
    Write-Warning "$TargetFolder が存在しません。zip化をスキップします。"
}

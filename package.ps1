param(
    [string]$TargetDir,
    [string]$FolderName
)

# ターゲットフォルダー作成
$TargetFolder = Join-Path $TargetDir $FolderName
if (-not $TargetFolder) {
    Write-Warning "ターゲットフォルダーのパスが正しくありません。"
    exit 1
}
New-Item -ItemType Directory -Path $TargetFolder -Force | Out-Null
Write-Host "Created $TargetFolder" -ForegroundColor Green

# Applicationフォルダー作成
$AppFolder = Join-Path $TargetFolder "Application"
New-Item -ItemType Directory -Path $AppFolder -Force | Out-Null
Write-Host "Created $AppFolder" -ForegroundColor Green

# engineフォルダー作成
$EngineFolder = Join-Path $TargetFolder "engine"
New-Item -ItemType Directory -Path $EngineFolder -Force | Out-Null
Write-Host "Created $EngineFolder" -ForegroundColor Green

# msbuildでC++ビルド
Write-Host "msbuildを開始しています..." -ForegroundColor Yellow
$msbuildProcess = Start-Process -FilePath "msbuild.exe" -ArgumentList "project/OriGine.sln /p:Configuration=Release" -NoNewWindow -Wait -PassThru
if ($msbuildProcess.ExitCode -ne 0) {
    Write-Warning "msbuildが失敗しました。終了コード: $($msbuildProcess.ExitCode)"
    exit 1
}
Write-Host "msbuildが完了しました。" -ForegroundColor Green

# .exeと.libをコピー
$buildOutput = "generated/output/Release"
if (Test-Path $buildOutput) {
    Get-ChildItem $buildOutput -Include *.exe,*.lib,*.dll -File | ForEach-Object {
        Copy-Item $_.FullName -Destination $TargetFolder -Force
        Write-Host "Copied $($_.Name) to $TargetFolder" -ForegroundColor Green
    }
} else {
    Write-Warning "$buildOutput が存在しません。ビルドに失敗している可能性があります。"
    exit 1
}

# Applicationリソースコピー
$sourceAppResource = "project/Application/resource"
if (Test-Path $sourceAppResource) {
    Copy-Item -Path "$sourceAppResource\*" -Destination $AppFolder -Recurse -Force
    Write-Host "Applicationリソースを $AppFolder にコピーしました。" -ForegroundColor Green
} else {
    Write-Warning "Applicationリソースフォルダー ($sourceAppResource) が存在しません。"
}

# engineリソースコピー
$sourceEngineResource = "project/engine/resource"
if (Test-Path $sourceEngineResource) {
    Get-ChildItem -Path $sourceEngineResource -Recurse | Where-Object {
        # logsフォルダーとその中身をすべて除外
        -not ($_.FullName -match [regex]::Escape((Join-Path $sourceEngineResource "logs")))
    } | ForEach-Object {
        $dest = $_.FullName.Replace((Resolve-Path $sourceEngineResource).Path, $EngineFolder)
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
    Write-Host "engineリソースを $EngineFolder にコピーしました。" -ForegroundColor Green
} else {
    Write-Warning "engineリソースフォルダー ($sourceEngineResource) が存在しません。"
}

# zip化
$zipPath = "${TargetFolder}.zip"
if (Test-Path $TargetFolder) {
    Compress-Archive -Path "$TargetFolder\*" -DestinationPath $zipPath -Force
    Write-Host "パッケージ化が完了しました: $zipPath"
} else {
    Write-Warning "$TargetFolder が存在しません。zip化をスキップします。"
}

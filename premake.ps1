# Premake5 を実行するPowerShellスクリプト

# ワークスペースフォルダーを取得（スクリプトの場所）
$workspaceFolder = Split-Path -Path $MyInvocation.MyCommand.Path -Parent

# Premake5の設定ディレクトリに移動
$configPath = Join-Path $workspaceFolder "project\config"

# ディレクトリが存在するかチェック
if (-Not (Test-Path $configPath)) {
    Write-Error "設定ディレクトリが見つかりません: $configPath"
    exit 1
}

try {
    # 設定ディレクトリに移動してPremake5を実行
    Set-Location $configPath
    Write-Host "Premake5を実行中... (ディレクトリ: $configPath)"

    & premake5 vs2022

    if ($LASTEXITCODE -eq 0) {
        Write-Host "Premake5の実行が完了しました。" -ForegroundColor Green
    }
    else {
        Write-Error "Premake5の実行中にエラーが発生しました。終了コード: $LASTEXITCODE"
        exit $LASTEXITCODE
    }
}
catch {
    Write-Error "エラーが発生しました: $($_.Exception.Message)"
    exit 1
}
finally {
    # 元のディレクトリに戻る
    Set-Location $workspaceFolder
}

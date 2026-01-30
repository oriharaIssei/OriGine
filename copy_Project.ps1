param(
    [Parameter(Mandatory = $true)]
    [string]$SourcePath, # コピー元のプロジェクトパス

    [Parameter(Mandatory = $true)]
    [string]$DestinationPath # コピー先のパス
)

# === 設定：除外するフォルダーとファイルのリスト ===
# Unityの自動生成フォルダ、隠しフォルダ、バージョン管理フォルダ
$ExcludeFolders = @(
    "Library",
    "Temp",
    "Obj",
    "Logs",
    "UserSettings",
    "Build",
    "Builds",
    "MemoryCaptures",
    "Recordings",
    ".git",
    ".svn",
    ".vs",
    ".gradle",
    ".idea"
)

# ルート直下にある不要なファイル（Visual Studioのソリューションファイルなど）
# ※ .meta ファイルは絶対に消さないように注意してください
$ExcludeExtensions = @(
    ".csproj",
    ".sln",
    ".suo",
    ".user",
    ".userprefs",
    ".pidb"
)

# === 関数定義 ===
function Copy-FilteredRecursive {
    param (
        [string]$CurrentSource,
        [string]$CurrentDest
    )

    # フォルダが存在しない場合は作成
    if (-not (Test-Path $CurrentDest)) {
        New-Item -ItemType Directory -Path $CurrentDest -Force | Out-Null
        Write-Host "作成: $CurrentDest" -ForegroundColor Cyan
    }

    # アイテムを取得 (隠しファイル .git なども含めるため -Force を使用)
    $items = Get-ChildItem -Path $CurrentSource -Force

    foreach ($item in $items) {
        # 1. フォルダの場合
        if ($item.PSIsContainer) {
            if ($ExcludeFolders -contains $item.Name) {
                Write-Host "除外: $($item.FullName)" -ForegroundColor DarkGray
                continue # このフォルダの中には入らずスキップ
            }

            # 再帰的にコピー処理
            $subDest = Join-Path $CurrentDest $item.Name
            Copy-FilteredRecursive -CurrentSource $item.FullName -CurrentDest $subDest
        }
        # 2. ファイルの場合
        else {
            if ($ExcludeExtensions -contains $item.Extension) {
                # 拡張子が除外リストにあればスキップ
                continue
            }

            # ファイルをコピー
            Copy-Item -Path $item.FullName -Destination $CurrentDest -Force
        }
    }
}

# === 実行部分 ===
try {
    Write-Host "=== Unityプロジェクトのクリーンコピーを開始します ===" -ForegroundColor Green
    Write-Host "元: $SourcePath"
    Write-Host "先: $DestinationPath"

    if (-not (Test-Path $SourcePath)) {
        throw "コピー元のパスが見つかりません。"
    }

    # 再帰コピー関数の呼び出し
    Copy-FilteredRecursive -CurrentSource $SourcePath -CurrentDest $DestinationPath

    Write-Host "`n完了しました。" -ForegroundColor Green
}
catch {
    Write-Error "エラーが発生しました: $_"
}

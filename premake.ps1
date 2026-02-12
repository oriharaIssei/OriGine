param(
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$RemainingArgs
)

$rootDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$premakeDir = Join-Path $rootDir "project\config"

$premakeExe = Join-Path $premakeDir "premake5.exe"
$premakeLua = Join-Path $premakeDir "premake5.lua"

# premake5.exe と premake5.lua の存在を確認
if (-not (Test-Path $premakeExe)) {
    Write-Host "Error: premake5.exe not found at $premakeExe" -ForegroundColor Red
    exit 1
}

if (-not (Test-Path $premakeLua)) {
    Write-Host "Error: premake5.lua not found at $premakeLua" -ForegroundColor Red
    exit 1
}
# 引数が無ければ vs2026 をデフォルトにする
if (-not $RemainingArgs -or $RemainingArgs.Count -eq 0) {
    $RemainingArgs = @("vs2026")
}

& $premakeExe --file=$premakeLua @RemainingArgs

if ($LASTEXITCODE -ne 0) {
    Write-Host "Premake failed." -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Host "Premake completed successfully." -ForegroundColor Green

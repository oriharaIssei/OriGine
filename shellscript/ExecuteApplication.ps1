# 指定した exeFile を 実行する
param(
    [string]$exeFile,
    [string]$WorkingDir = ".",
    [string[]]$Arguments = @()
)

Set-Location -Path $WorkingDir
& $exeFile @Arguments
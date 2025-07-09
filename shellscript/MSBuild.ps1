# msbuild を実行するだけのコマンド
param(
    [string]$TargetDir,
    [string]$ProjectName = "OriGine.sln",
    [string]$Configuration = "Release", 
    [string]$Profile = "x64"
)

msbuild $TargetDir/$ProjectName /p:Configuration=$Configuration /p:Platform=$Profile

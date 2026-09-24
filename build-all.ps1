# build-all.ps1 - 一键：编 C++ DLL + 发布 C# exe
$ErrorActionPreference = "Stop"

$root = "D:\桌面\BiliViewer"
Set-Location $root

Write-Host "=== 1/3 用 CMake 编译 C++ DLL ===" -ForegroundColor Cyan
if (-not (Test-Path "$root\cpp-build")) {
    cmake -B cpp-build -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER=g++
}
cmake --build cpp-build --target BiliNative

if (-not (Test-Path "$root\cpp-build\BiliNative.dll")) {
    throw "BiliNative.dll 编译失败"
}
Write-Host "  → cpp-build\BiliNative.dll 已生成" -ForegroundColor Green

Write-Host "`n=== 2/3 发布 C# ===" -ForegroundColor Cyan
Set-Location "$root\csharp\BiliAnalyzer"
dotnet publish -c Release

$pub = "$root\csharp\BiliAnalyzer\bin\Release\net10.0-windows\win-x64\publish"
if (-not (Test-Path $pub)) {
    throw "publish 目录不存在"
}

Write-Host "`n=== 3/3 打包 ===" -ForegroundColor Cyan
$zip = "$root\BiliAnalyzer-v1.0.zip"
if (Test-Path $zip) { Remove-Item $zip }
Compress-Archive -Path "$pub\*" -DestinationPath $zip

Write-Host "`n=== 完成 ===" -ForegroundColor Green
Write-Host "exe 目录：$pub"
Write-Host "压缩包：$zip"
ls $pub
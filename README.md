# BiliViewer

B 站历史记录分析工具。抓取个人观看历史，按日期筛选、统计观看时长、计算完成率。

> ⚠️ **仅供个人学习使用。** 请遵守 B 站用户协议，不要公开传播或商业化。

---

## 功能

- 从 B 站抓取个人历史记录（需登录）
- 从本地文件读取历史记录
- 按日期范围筛选
- 统计观看时长
- 计算平均观看完成率
- 内嵌 WebView2 登录，自动抓取 Cookie
- 支持"记住我"，下次自动登录
- 发布为免安装 .NET 的独立 exe

---

## 技术栈

| 层 | 技术 |
|----|------|
| 界面 | C# / WinForms / .NET 10 |
| 核心逻辑 | C++17 |
| 跨语言 | P/Invoke |
| 构建 | CMake + MSBuild |
| 登录 | WebView2 |
| 网络 | libcurl |
| JSON | cJSON（C++）/ System.Text.Json（C#） |

---

## 构建

### 环境要求

- Windows 10/11 x64
- .NET 10 SDK
- MSYS2（MinGW-w64）
- CMake 3.20+
- libcurl、cjson（通过 MSYS2 安装）

### 安装依赖（MSYS2）

```bash
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-cmake
pacman -S mingw-w64-x86_64-curl
pacman -S mingw-w64-x86_64-cjson
一键构建
powershell
.\build-all.ps1
输出：

cpp-build\BiliNative.dll — C++ 核心

csharp\BiliAnalyzer\bin\Release\net10.0-windows\win-x64\publish\ — 最终 exe

手动构建
powershell
# 1. 编 C++ DLL
cmake -B cpp-build -G "MinGW Makefiles"
cmake --build cpp-build --target BiliNative

# 2. 发布 C#
cd csharp\BiliAnalyzer
dotnet publish -c Release
使用
双击 BiliAnalyzer.exe

首次使用点"抓取网络"，弹出登录窗口

在 WebView2 中登录 B 站

登录成功后自动保存 Cookie

等待抓取完成，数据自动保存到 data/biliHistory

之后：

勾选"记住我"，下次自动登录

点"读取本地"，从本地文件读数据

点"按日期筛选"、"算时长"、"完成率"分析

数据格式
data/biliHistory 每行一条记录，| 分隔：

text
时间戳|标题|分区|BVID|类型|已看秒数|总时长
注意： 标题和分区里的 | 会被替换为 ;，防止破坏格式。

合规说明
本项目仅供个人学习使用。

使用 Cookie 模拟请求抓取 B 站数据，违反 B 站用户协议

不要公开传播、商业化、或上传到应用商店

Cookie 只存本地，不要上传到服务器

建议使用 WebView2 登录（用户主动授权），而非读取浏览器数据库

如果 B 站要求下架，本项目会立即停止维护。

已知问题
仅支持 Windows x64

仅支持 B 站历史记录 API

Cookie 过期后需重新登录

部分标题含特殊字符可能显示异常

开发笔记
CSAPP 第 7 章 — 理解链接和动态库

CLR via C# — 理解 P/Invoke 和 GC

P/Invoke 官方文档

License
个人学习项目，未声明开源协议。 请勿用于商业用途。

致谢
libcurl

cJSON

WebView2

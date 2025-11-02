@echo off
chcp 65001 >nul
echo ============================================================
echo       Breakpad vs Native MiniDump 对比测试
echo ============================================================
echo.

REM 设置路径
set BUILD_DIR=build\bin\Debug
set DUMP_DIR=%BUILD_DIR%\crash_dumps

REM 清理旧的 dump 文件
echo [步骤 1] 清理旧的 dump 文件...
if exist "%DUMP_DIR%" (
    del /Q "%DUMP_DIR%\*" 2>nul
    echo   已清理旧文件
) else (
    mkdir "%DUMP_DIR%" 2>nul
    echo   创建 dump 目录
)
echo.

REM 测试 Breakpad
echo [步骤 2] 测试 Google Breakpad (空指针崩溃)...
echo -----------------------------------------------------------
"%BUILD_DIR%\crash_dump_test.exe" 1
echo.
timeout /t 2 >nul

REM 测试原生 MiniDump
echo [步骤 3] 测试 Windows 原生 MiniDump (空指针崩溃)...
echo -----------------------------------------------------------
"%BUILD_DIR%\crash_dump_native_test.exe" 1
echo.
timeout /t 2 >nul

REM 显示生成的文件
echo [步骤 4] 查看生成的 dump 文件...
echo -----------------------------------------------------------
dir /B "%DUMP_DIR%\*.dmp" 2>nul
echo.

REM 对比文件大小
echo [步骤 5] 对比 dump 文件信息...
echo -----------------------------------------------------------
for %%f in ("%DUMP_DIR%\*.dmp") do (
    echo 文件: %%~nxf
    echo 大小: %%~zf 字节
    echo.
)

echo ============================================================
echo 测试完成！
echo.
echo 你可以使用以下工具打开 dump 文件进行对比:
echo   - Visual Studio: 文件 ^> 打开 ^> 文件 ^> 选择 .dmp
echo   - WinDbg: windbg -z ^<dump文件路径^>
echo.
echo Breakpad dump 文件通常以 UUID 命名
echo 原生 dump 文件通常以 native_crash_时间戳 命名
echo ============================================================
pause

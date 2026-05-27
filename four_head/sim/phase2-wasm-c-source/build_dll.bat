@echo off
REM ============================================================================
REM  EMC Logic DLL 编译脚本
REM ============================================================================

setlocal

REM 设置编译器路径
set GCC=D:\mingw64\mingw64\bin\gcc.exe

REM 获取当前目录
cd /d "%~dp0"

echo ========================================
echo   EMC Logic DLL Build
echo ========================================
echo.

REM 检查编译器
if not exist "%GCC%" (
    echo [ERROR] GCC compiler not found at %GCC%
    echo Please install MinGW64 or update the GCC path in this script.
    pause
    exit /b 1
)

echo [INFO] Compiler: %GCC%
echo.

REM 编译DLL
echo [STEP 1] Compiling emc_logic.dll...
"%GCC%" -shared ^
    -o "..\emc_logic.dll" ^
    "..\c_logic\emc_logic.c" ^
    "dll_wrapper.c" ^
    "-Wl,--out-implib,..\libemc_logic.a" ^
    -O2 ^
    -Wall ^
    -DEMC_LOGIC_EXPORTS ^
    "-I..\c_logic" ^
    -include "dll_wrapper.h"

if %ERRORLEVEL% neq 0 (
    echo.
    echo [ERROR] Compilation failed!
    pause
    exit /b 1
)

echo [SUCCESS] DLL compiled successfully!
echo.

REM 检查输出文件
if exist "..\emc_logic.dll" (
    for %%A in ("..\emc_logic.dll") do set SIZE=%%~zA
    echo   [OK] emc_logic.dll (!SIZE! bytes)
) else (
    echo   [FAIL] emc_logic.dll not found!
)

if exist "..\libemc_logic.a" (
    for %%A in ("..\libemc_logic.a") do set SIZE=%%~zA
    echo   [OK] libemc_logic.a (!SIZE! bytes)
)

echo.
echo ========================================
echo   Build Complete!
echo ========================================
echo.
echo Next steps:
echo   1. Run tests: cd ..\tests ^&^& python test_dll_connection.py
echo   2. Start GUI: cd ..\python_adapter ^&^& python emc_sim_gui.py
echo.

pause
endlocal

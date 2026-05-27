@echo off
chcp 65001 >nul

:: Change to script directory (ensure relative paths work)
cd /d "%~dp0"

echo ========================================
echo   EMC 面板模拟器 - 启动服务器
echo ========================================
echo.
echo 正在启动HTTP服务器...
echo.
echo 请在浏览器中访问:
echo http://localhost:8080/index.html
echo.
echo 按 Ctrl+C 停止服务器
echo.
echo ========================================
echo.

python -m http.server 8080

pause

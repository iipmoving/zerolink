@echo off
chcp 65001 >nul
echo ========================================
echo EMC Framework Web版 - 启动服务器
echo ========================================
echo.
echo 正在启动本地HTTP服务器...
echo 访问地址: http://localhost:8080
echo.
echo 按 Ctrl+C 停止服务器
echo.

cd /d "%~dp0phase1-js-logic"
python -m http.server 8080

pause

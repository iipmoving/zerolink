@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

:: Change to script directory (ensure relative paths work)
cd /d "%~dp0"

echo ========================================
echo   EMC Panel Simulator - Auto Start
echo ========================================
echo.

:: Set port
set PORT=8080
set URL=http://localhost:%PORT%/index.html

:: Check if Python is installed
where python >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] Python not found. Please install Python 3.x
    echo.
    echo Download: https://www.python.org/downloads/
    echo Check "Add Python to PATH" during installation
    echo.
    pause
    exit /b 1
)

echo [1/3] Checking if port %PORT% is in use...

:: Check if port is in use
netstat -ano | findstr ":%PORT% " >nul 2>&1
if %errorlevel% equ 0 (
    echo [INFO] Port %PORT% is already in use, using existing service
    echo.
    goto OPEN_BROWSER
) else (
    echo [INFO] Port %PORT% is available, starting HTTP server...
    echo.
)

:: Start HTTP server in background
echo [2/3] Starting HTTP server (background)...
start "EMC HTTP Server" /MIN python -m http.server %PORT%

:: Wait for server to start
echo [INFO] Waiting for server to start...
timeout /t 2 /nobreak >nul

:: Verify server status
echo [INFO] Verifying server status...
timeout /t 1 /nobreak >nul

:OPEN_BROWSER
echo.
echo [3/3] Opening browser...
echo.
echo ========================================
echo   Server URL: %URL%
echo ========================================
echo.
echo [INFO] Server is running in background
echo [INFO] Closing this window will NOT stop the server
echo [INFO] To stop server, end python.exe in Task Manager
echo.

:: Open browser automatically
start "" "%URL%"

echo.
echo ========================================
echo   Start Complete!
echo ========================================
echo.

:: Exit without pause
exit /b 0

@echo off
chcp 65001 >nul
REM ============================================================================
REM EMC Logic Comparison Test - Quick Start Script
REM ============================================================================

echo ========================================
echo   EMC Logic Comparison Test Tool
echo ========================================
echo.

REM 检查Python
where python >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Python not found!
    echo Please install Python 3.7+
    pause
    exit /b 1
)

echo [STEP 1] Installing dependencies...
pip install flask flask-cors >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo [WARNING] pip install failed, trying with --user
    pip install --user flask flask-cors
)
echo [OK] Dependencies installed
echo.

echo [STEP 2] Starting backend server...
start "EMC DLL Server" cmd /k "cd /d %~dp0 && python dll_server.py"
echo [OK] Backend server started in new window
echo.

echo Waiting for server to start...
timeout /t 3 /nobreak >nul

echo [STEP 3] Opening test page...
start "" "%~dp0comparison_test.html"
if %ERRORLEVEL% neq 0 (
    echo [INFO] Please manually open: comparison_test.html
)
echo [OK] Test page opened
echo.

echo ========================================
echo   Ready!
echo ========================================
echo.
echo Backend: http://localhost:5000
echo Frontend: comparison_test.html
echo.
echo Instructions:
echo   1. Click "Initialize" button
echo   2. Run test cases
echo   3. Check results
echo.
echo Press any key to stop backend server...
pause >nul

REM Stop backend server
taskkill /FI "WINDOWTITLE eq EMC DLL Server" /T /F >nul 2>&1
echo Backend server stopped.

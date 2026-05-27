@echo off
chcp 65001 >nul
cd /d "%~dp0"

:: 检查依赖
python -c "from pymodbus.client import ModbusSerialClient" 2>nul
if errorlevel 1 (
    echo [安装依赖...]
    python -m pip install pymodbus pyserial -q
    echo.
)

:: 启动 GUI
echo 启动 M4 MODBUS 调试工具...
python "%~dp0m4_gui.py"
pause

@echo off
REM === Keyboard_Motor 一键烧录 (J-Link SWD) ===
REM 接线: J-Link SWDIO->PA19, SWCLK->PA20, GND->GND, VTref->3V3
cd /d "%~dp0"
JLink.exe -device MSPM0G3507 -if SWD -speed 4000 -autoconnect 1 -ExitOnError 1 -CommanderScript flash_jlink.jlink
echo.
echo === 烧录结束, 看到 "PC = 0x000xxxxx" 即运行中 ===
pause

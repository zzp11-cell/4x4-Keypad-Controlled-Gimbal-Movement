@echo off
:: Local SysConfig wrapper for Yuntai1_MSPM0 (天猛星 MSPM0G3507 云台项目)
::
:: 用 C:/ti 新版 SysConfig 1.26.2 + MSPM0 SDK 2.10.00.04 生成 ti_msp_dl_config.c/.h
::
:: Called by .uvprojx BeforeMake as:
::   run_syscfg.bat "<proj_dir=keil\>" "<syscfg=../empty.syscfg>"
::
:: Output goes to project root (keil\.. = Yuntai1_MSPM0\) because the .uvprojx
:: references "..\ti_msp_dl_config.c" (root copy), so files must be generated there.
::
:: 若 SysConfig 安装路径不同, 改下方 SYSCFG_CLI / SDK_PRODUCT 两行即可.

setlocal

set SYSCFG_CLI="C:\ti\sysconfig_1.26.2\sysconfig_cli.bat"
set SDK_PRODUCT="C:\ti\mspm0_sdk_2_10_00_04\.metadata\product.json"

if not exist %SYSCFG_CLI% (
    echo [run_syscfg] Couldn't find Sysconfig CLI: %SYSCFG_CLI%
    echo [run_syscfg] 请确认 SysConfig 安装路径, 或改用 SysConfig GUI 打开 empty.syscfg 生成.
    exit /b 1
)
if not exist %SDK_PRODUCT% (
    echo [run_syscfg] Couldn't find SDK product.json: %SDK_PRODUCT%
    exit /b 1
)

set PROJ_DIR=%~1
set SYSCFG_FILE=%~2

:: Run from the project dir (keil\). -o ".." writes generated files into the
:: project root (keil\.. = Yuntai1_MSPM0\), which is where the .uvprojx expects them.
pushd "%PROJ_DIR%"

echo [run_syscfg] Running SysConfig: %SYSCFG_FILE% -^> (root)
%SYSCFG_CLI% -o ".." -s %SDK_PRODUCT% --compiler keil "%SYSCFG_FILE%"

popd
endlocal

@echo off
setlocal enabledelayedexpansion

set "SOURCE_DIR=..\shaders"
set "OUTPUT_DIR=..\shaders\bin"

:: 确保输出目录存在
if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

for %%f in ("%SOURCE_DIR%\*.frag.hlsl") do (
    if exist "%%f" (
        set "fullpath=%%f"
        set "filename=%%~nxf"
        set "basename=!filename:.hlsl=!"
        
        shadercross "%%f" -o "!OUTPUT_DIR!\!basename!.spv"
        shadercross "%%f" -o "!OUTPUT_DIR!\!basename!.msl"
        shadercross "%%f" -o "!OUTPUT_DIR!\!basename!.dxil"
    )
)

endlocal
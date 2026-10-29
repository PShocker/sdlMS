@echo off
rem Run clang-tidy over the project with the settings from .clang-tidy.
rem
rem   scripts\clang-tidy.bat                     -- check every source file
rem   scripts\clang-tidy.bat src\client\main.cpp
rem
rem The extra arguments teach clang-tidy about the MinGW toolchain that CMake
rem uses; override SDLMS_MINGW_ROOT when GCC lives somewhere else.
setlocal enabledelayedexpansion

set "ROOT=%~dp0.."
pushd "%ROOT%"
set "BUILD=%ROOT%\build"
if not defined SDLMS_MINGW_ROOT set "SDLMS_MINGW_ROOT=C:/mingw64"

if not exist "%BUILD%\compile_commands.json" (
  echo [clang-tidy] %BUILD%\compile_commands.json is missing.
  echo [clang-tidy] configure the project first:
  echo [clang-tidy]   cmake -S . -B build -G Ninja
  popd
  exit /b 1
)

if not "%~1"=="" goto :one

set "STATUS=0"
for /r "%ROOT%\src" %%f in (*.cpp) do (
  clang-tidy -p "%BUILD%" --config-file="%ROOT%\.clang-tidy" --extra-arg-before=--target=x86_64-w64-windows-gnu --extra-arg-before=--gcc-toolchain=%SDLMS_MINGW_ROOT% --extra-arg-before=-Wno-unused-command-line-argument "%%f" >nul 2>&1
  if !ERRORLEVEL! NEQ 0 (
    echo [clang-tidy] findings in %%f
    clang-tidy -p "%BUILD%" --config-file="%ROOT%\.clang-tidy" --extra-arg-before=--target=x86_64-w64-windows-gnu --extra-arg-before=--gcc-toolchain=%SDLMS_MINGW_ROOT% --extra-arg-before=-Wno-unused-command-line-argument "%%f"
    set "STATUS=1"
  )
)
popd
exit /b !STATUS!

:one
clang-tidy -p "%BUILD%" --config-file="%ROOT%\.clang-tidy" --extra-arg-before=--target=x86_64-w64-windows-gnu --extra-arg-before=--gcc-toolchain=%SDLMS_MINGW_ROOT% --extra-arg-before=-Wno-unused-command-line-argument %*
set "STATUS=%ERRORLEVEL%"
popd
exit /b %STATUS%

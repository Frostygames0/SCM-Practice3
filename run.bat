@echo off
setlocal
cd /d "%~dp0"

set "source=%~1"
if "%source%"=="" set "source=examples\vector_min_1.asm"
set "first=%~2"
if "%first%"=="" set "first=0"
set "last=%~3"
if "%last%"=="" set "last=4"
for %%f in ("%source%") do set "name=%%~nf"

cmake -S . -B build || exit /b 1
cmake --build build --config Release || exit /b 1
if not exist out mkdir out

build\bin\asm.exe -c "%source%" -r "out\%name%.bin" -t || exit /b 1
build\bin\vm.exe -p "out\%name%.bin" -d "out\%name%.xml" -r %first% %last% || exit /b 1
type "out\%name%.xml"

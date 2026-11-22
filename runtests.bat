@echo off
setlocal
cd /d "%~dp0"

cmake -S . -B build || exit /b 1
cmake --build build --config Release || exit /b 1
ctest --test-dir build -C Release --output-on-failure || exit /b 1

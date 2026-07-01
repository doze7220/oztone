@echo off
REM Load Visual Studio environment variables
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

REM Run build with CMake
cmake -B build
cmake --build build --config Release

@echo off
REM Visual Studioの環境変数を読み込む
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

REM CMakeでビルドを実行する
cmake -B build
cmake --build build --config Release

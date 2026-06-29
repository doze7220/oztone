cmake_minimum_required(VERSION 3.10)
project(OZtone VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ソースファイル
set(SOURCES
    src/main.cpp
    src/Application.cpp
    src/Window.cpp
)

set(HEADERS
    src/Application.h
    src/Window.h
)

# 実行ファイルの作成 (WIN32 を指定してコンソールウィンドウを非表示にする)
add_executable(OZtone WIN32 ${SOURCES} ${HEADERS})

# 必要なWin32ライブラリをリンク
target_link_libraries(OZtone PRIVATE user32.lib gdi32.lib)

if(MSVC)
    target_compile_options(OZtone PRIVATE /utf-8)
endif()

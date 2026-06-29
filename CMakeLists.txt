cmake_minimum_required(VERSION 3.10)
project(OZtone VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ソースファイル
set(SOURCES
    src/main.cpp
    src/Application.cpp
    src/Window.cpp
    src/ConfigManager.cpp
    src/Renderer.cpp
    src/AudioPlayer.cpp
    src/OZtone.rc
)

set(HEADERS
    src/Application.h
    src/Window.h
    src/ConfigManager.h
    src/Renderer.h
    src/AudioPlayer.h
)

# 実行ファイルの作成 (WIN32 を指定してコンソールウィンドウを非表示にする)
add_executable(OZtone WIN32 ${SOURCES} ${HEADERS})

# 必要なWin32ライブラリをリンク
target_link_libraries(OZtone PRIVATE user32.lib gdi32.lib d3d11.lib d2d1.lib dxgi.lib windowscodecs.lib dwrite.lib dcomp.lib)

if(MSVC)
    target_compile_options(OZtone PRIVATE /utf-8)
endif()

# テスト用にassetsフォルダの画像をビルド出力ディレクトリにコピー
add_custom_command(TARGET OZtone POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/assets/app_logo.png"
    "$<TARGET_FILE_DIR:OZtone>/app_logo.png"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/assets/app_logo_hover.png"
    "$<TARGET_FILE_DIR:OZtone>/app_logo_hover.png"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/assets/placeholder_art.png"
    "$<TARGET_FILE_DIR:OZtone>/placeholder_art.png"
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:OZtone>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/assets/test.mp3"
    "$<TARGET_FILE_DIR:OZtone>/assets/test.mp3"
)

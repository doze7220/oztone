cmake_minimum_required(VERSION 3.10)
project(OZtone VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_BINDINGS OFF CACHE BOOL "" FORCE)
set(WITH_ZLIB OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    taglib
    GIT_REPOSITORY https://github.com/taglib/taglib.git
    GIT_TAG v2.0.2
)
FetchContent_MakeAvailable(taglib)

# ソースファイル
set(SOURCES
    src/main.cpp
    src/Application.cpp
    src/Window.cpp
    src/ConfigManager.cpp
    src/Renderer.cpp
    src/LayoutCalculator.cpp
    src/Visualizer.cpp
    src/Visualizer_PrismBeat.cpp
    src/Visualizer_HaloDust.cpp
    src/Widgets.cpp
    src/PlaylistWidget.cpp
    src/PlaybackControlsWidget.cpp
    src/VolumeControlWidget.cpp
    src/LogoMenuWidget.cpp
    src/OsdWidget.cpp
    src/AudioPlayer.cpp
    src/TagManager.cpp
    src/PlaylistManager.cpp
    src/OZtone.rc
)

set(HEADERS
    src/Application.h
    src/Window.h
    src/ConfigManager.h
    src/Renderer.h
    src/LayoutCalculator.h
    src/Visualizer.h
    src/IVisualizerStyle.h
    src/Visualizer_PrismBeat.h
    src/Visualizer_HaloDust.h
    src/WidgetContext.h
    src/Widget.h
    src/Widgets.h
    src/PlaylistWidget.h
    src/PlaybackControlsWidget.h
    src/VolumeControlWidget.h
    src/LogoMenuWidget.h
    src/OsdWidget.h
    src/AudioPlayer.h
    src/TagManager.h
    src/PlaylistManager.h
)

# 実行ファイルの作成 (WIN32 を指定してコンソールウィンドウを非表示にする)
add_executable(OZtone WIN32 ${SOURCES} ${HEADERS})

target_include_directories(OZtone PRIVATE 
    ${CMAKE_CURRENT_BINARY_DIR}/_deps/taglib-src
    ${CMAKE_CURRENT_BINARY_DIR}/_deps/taglib-src/taglib
    ${CMAKE_CURRENT_BINARY_DIR}/_deps/taglib-src/taglib/toolkit
    ${CMAKE_CURRENT_BINARY_DIR}/_deps/taglib-src/taglib/mpeg
    ${CMAKE_CURRENT_BINARY_DIR}/_deps/taglib-src/taglib/mpeg/id3v2
    ${CMAKE_CURRENT_BINARY_DIR}/_deps/taglib-src/taglib/mpeg/id3v2/frames
)

target_compile_definitions(OZtone PRIVATE TAGLIB_STATIC)

# 必要なWin32ライブラリをリンク
target_link_libraries(OZtone PRIVATE user32.lib gdi32.lib d3d11.lib d2d1.lib dxgi.lib windowscodecs.lib dwrite.lib dcomp.lib tag)

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
)

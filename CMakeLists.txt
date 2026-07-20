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
    src/Application_Initialize.cpp
    src/Application_Playback.cpp
    src/Application_Playlist.cpp
    src/Application_FileDrop.cpp
    src/Application_Render.cpp
    src/Window_Proc.cpp
    src/Window_Initialize.cpp
    src/Window_Mouse.cpp
    src/Window_TrayMenu.cpp
    src/Window_DropTarget.cpp
    src/Window_System.cpp
    src/ConfigManager.cpp
    src/ConfigManager_Window.cpp
    src/ConfigManager_Playlist.cpp
    src/ConfigManager_Playback.cpp
    src/ConfigManager_LogoMenu.cpp
    src/ConfigManager_Visualizer.cpp
    src/ConfigManager_System.cpp
    src/Renderer.cpp
    src/Renderer_Initialize.cpp
    src/Renderer_Image.cpp
    src/Renderer_Update.cpp
    src/Renderer_Draw.cpp
    src/Renderer_Context.cpp
    src/Renderer_TrackDrum.cpp
    src/LayoutCalculator.cpp
    src/Visualizer.cpp
    src/Visualizer_PrismBeat.cpp
    src/Visualizer_HaloDust.cpp
    src/Widget_AppLogo.cpp
    src/Widget_TrackInfo.cpp
    src/Widget_SeekBar.cpp
    src/Widget_GlobalHotkeys.cpp
    src/Widget_ResizeGrip.cpp
    src/WidgetCommon.cpp
    src/Widget_Playlist.cpp
    src/Widget_Playlist_Resources.cpp
    src/Widget_Playlist_Update.cpp
    src/Widget_Playlist_Toolbar.cpp
    src/Widget_Playlist_DrawItems.cpp
    src/Widget_Playlist_Draw.cpp
    src/Widget_PlaybackControls.cpp
    src/Widget_VolumeControl.cpp
    src/Widget_LogoMenu.cpp
    src/Widget_Osd.cpp
    src/AudioPlayer.cpp
    src/AudioAnalyzer.cpp
    src/AudioManager.cpp
    src/FileManager.cpp
    src/TrackDatabase.cpp
    src/TrackAnalyzer.cpp
    src/PlaylistManager.cpp
    src/ArtFramingDatabase.cpp
    src/ThumbnailDatabase.cpp
    src/ThumbCacher.cpp
    src/OZtone.rc
)

set(HEADERS
    src/Application.h
    src/Window.h
    src/ConfigManager.h
    src/Renderer.h
    src/Renderer_TrackDrum.h
    src/LayoutCalculator.h
    src/Visualizer.h
    src/IVisualizerStyle.h
    src/Visualizer_PrismBeat.h
    src/Visualizer_HaloDust.h
    src/WidgetContext.h
    src/Widget.h
    src/Widget_AppLogo.h
    src/Widget_TrackInfo.h
    src/Widget_SeekBar.h
    src/Widget_GlobalHotkeys.h
    src/WidgetCommon.h
    src/Widget_Playlist.h
    src/Widget_PlaybackControls.h
    src/Widget_VolumeControl.h
    src/Widget_LogoMenu.h
    src/Widget_Osd.h
    src/AudioPlayer.h
    src/AudioAnalyzer.h
    src/AudioManager.h
    src/FileManager.h
    src/TrackDatabase.h
    src/TrackAnalyzer.h
    src/PlaylistManager.h
    src/ArtFramingDatabase.h
    src/ThumbnailDatabase.h
    src/ThumbCacher.h
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

#include "ConfigManager.h"
#include <vector>

ConfigManager::ConfigManager()
    : m_showTitleBar(false), m_showWindowFrame(false), m_showTaskbar(false),
      m_windowX(CW_USEDEFAULT), m_windowY(CW_USEDEFAULT), m_windowWidth(1024), m_windowHeight(512),
      m_logoX(16), m_logoY(16), m_logoWidth(64), m_logoHeight(64),
      m_baseX(30), m_baseY(350), m_artOffsetX(0), m_artOffsetY(0), m_artSize(120), m_bgOpacity(0.3f),
      m_fallbackArtOpacity(0.5f),
      m_titleOffsetX(140), m_titleOffsetY(10), m_titleFontSize(32.0f),
      m_artistOffsetX(140), m_artistOffsetY(55), m_artistFontSize(18.0f),
      m_seekBarWidthRatio(0.95f), m_seekBarHeight(3), m_seekBarBottomOffset(50),
      m_seekBarBgOpacity(0.3f), m_seekBarTimeFontFamily(L"Consolas"),
      m_seekBarTimeFontSize(12.0f), m_seekBarTimeAreaWidth(100),
      m_seekBarTimeLetterSpacing(0.0f),
      m_nextBaseRightOffset(250), m_nextBaseBottomOffset(80),
      m_nextArtOffsetX(0), m_nextArtOffsetY(0), m_nextArtSize(40),
      m_nextBgOpacity(0.3f), m_nextFallbackArtOpacity(0.5f),
      m_nextLabelOffsetX(0), m_nextLabelOffsetY(-20), m_nextLabelFontSize(12.0f),
      m_nextTitleOffsetX(50), m_nextTitleOffsetY(0), m_nextTitleFontSize(14.0f),
      m_nextArtistOffsetX(50), m_nextArtistOffsetY(20), m_nextArtistFontSize(12.0f) {
}

ConfigManager::~ConfigManager() {}

bool ConfigManager::Initialize() {
    std::wstring exePath = GetExecutablePath();
    if (exePath.empty()) {
        return false;
    }

    // 実行ファイルと同階層の OZtone.ini を指すようにパスを構築
    size_t pos = exePath.find_last_of(L"\\/");
    if (pos != std::wstring::npos) {
        m_iniFilePath = exePath.substr(0, pos) + L"\\OZtone.ini";
    } else {
        m_iniFilePath = L"OZtone.ini";
    }

    // ファイルが存在するか確認
    DWORD dwAttrib = GetFileAttributesW(m_iniFilePath.c_str());
    if (dwAttrib == INVALID_FILE_ATTRIBUTES) {
        // 存在しない場合はデフォルト設定を保存
        SaveDefaultSettings();
    }

    // 設定を読み込む
    LoadSettings();

    return true;
}

std::wstring ConfigManager::GetExecutablePath() const {
    std::vector<wchar_t> buffer(MAX_PATH);
    DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (length == 0) {
        return L"";
    }
    return std::wstring(buffer.data(), length);
}

void ConfigManager::LoadSettings() {
    m_showTitleBar = GetPrivateProfileIntW(L"Window", L"ShowTitleBar", 0, m_iniFilePath.c_str()) != 0;
    m_showWindowFrame = GetPrivateProfileIntW(L"Window", L"ShowWindowFrame", 0, m_iniFilePath.c_str()) != 0;
    m_showTaskbar = GetPrivateProfileIntW(L"Window", L"ShowTaskbar", 0, m_iniFilePath.c_str()) != 0;

    m_windowX = GetPrivateProfileIntW(L"Window", L"WindowX", CW_USEDEFAULT, m_iniFilePath.c_str());
    m_windowY = GetPrivateProfileIntW(L"Window", L"WindowY", CW_USEDEFAULT, m_iniFilePath.c_str());
    m_windowWidth = GetPrivateProfileIntW(L"Window", L"WindowWidth", 1024, m_iniFilePath.c_str());
    m_windowHeight = GetPrivateProfileIntW(L"Window", L"WindowHeight", 512, m_iniFilePath.c_str());

    m_logoX = GetPrivateProfileIntW(L"Layout_AppLogo", L"X", 16, m_iniFilePath.c_str());
    m_logoY = GetPrivateProfileIntW(L"Layout_AppLogo", L"Y", 16, m_iniFilePath.c_str());
    m_logoWidth = GetPrivateProfileIntW(L"Layout_AppLogo", L"Width", 64, m_iniFilePath.c_str());
    m_logoHeight = GetPrivateProfileIntW(L"Layout_AppLogo", L"Height", 64, m_iniFilePath.c_str());

    m_baseX = GetPrivateProfileIntW(L"Layout_NowPlaying", L"BaseX", 30, m_iniFilePath.c_str());
    m_baseY = GetPrivateProfileIntW(L"Layout_NowPlaying", L"BaseY", 350, m_iniFilePath.c_str());
    m_artOffsetX = GetPrivateProfileIntW(L"Layout_NowPlaying", L"ArtOffsetX", 0, m_iniFilePath.c_str());
    m_artOffsetY = GetPrivateProfileIntW(L"Layout_NowPlaying", L"ArtOffsetY", 0, m_iniFilePath.c_str());
    m_artSize = GetPrivateProfileIntW(L"Layout_NowPlaying", L"ArtSize", 120, m_iniFilePath.c_str());

    wchar_t buf[32];
    GetPrivateProfileStringW(L"Layout_NowPlaying", L"BgOpacity", L"0.3", buf, 32, m_iniFilePath.c_str());
    try { m_bgOpacity = std::stof(buf); } catch (...) { m_bgOpacity = 0.3f; }

    GetPrivateProfileStringW(L"Layout_NowPlaying", L"FallbackArtOpacity", L"0.5", buf, 32, m_iniFilePath.c_str());
    try { m_fallbackArtOpacity = std::stof(buf); } catch (...) { m_fallbackArtOpacity = 0.5f; }

    m_titleOffsetX = GetPrivateProfileIntW(L"Layout_NowPlaying", L"TitleOffsetX", 140, m_iniFilePath.c_str());
    m_titleOffsetY = GetPrivateProfileIntW(L"Layout_NowPlaying", L"TitleOffsetY", 10, m_iniFilePath.c_str());
    GetPrivateProfileStringW(L"Layout_NowPlaying", L"TitleFontSize", L"32.0", buf, 32, m_iniFilePath.c_str());
    try { m_titleFontSize = std::stof(buf); } catch (...) { m_titleFontSize = 32.0f; }

    m_artistOffsetX = GetPrivateProfileIntW(L"Layout_NowPlaying", L"ArtistOffsetX", 140, m_iniFilePath.c_str());
    m_artistOffsetY = GetPrivateProfileIntW(L"Layout_NowPlaying", L"ArtistOffsetY", 55, m_iniFilePath.c_str());
    GetPrivateProfileStringW(L"Layout_NowPlaying", L"ArtistFontSize", L"18.0", buf, 32, m_iniFilePath.c_str());
    try { m_artistFontSize = std::stof(buf); } catch (...) { m_artistFontSize = 18.0f; }

    GetPrivateProfileStringW(L"Layout_SeekBar", L"WidthRatio", L"0.95", buf, 32, m_iniFilePath.c_str());
    try { m_seekBarWidthRatio = std::stof(buf); } catch (...) { m_seekBarWidthRatio = 0.95f; }
    
    m_seekBarHeight = GetPrivateProfileIntW(L"Layout_SeekBar", L"Height", 3, m_iniFilePath.c_str());
    m_seekBarBottomOffset = GetPrivateProfileIntW(L"Layout_SeekBar", L"BottomOffset", 50, m_iniFilePath.c_str());
    
    GetPrivateProfileStringW(L"Layout_SeekBar", L"BgOpacity", L"0.3", buf, 32, m_iniFilePath.c_str());
    try { m_seekBarBgOpacity = std::stof(buf); } catch (...) { m_seekBarBgOpacity = 0.3f; }
    
    GetPrivateProfileStringW(L"Layout_SeekBar", L"TimeFontFamily", L"Consolas", buf, 32, m_iniFilePath.c_str());
    m_seekBarTimeFontFamily = buf;
    
    GetPrivateProfileStringW(L"Layout_SeekBar", L"TimeFontSize", L"12.0", buf, 32, m_iniFilePath.c_str());
    try { m_seekBarTimeFontSize = std::stof(buf); } catch (...) { m_seekBarTimeFontSize = 12.0f; }
    
    m_seekBarTimeAreaWidth = GetPrivateProfileIntW(L"Layout_SeekBar", L"TimeAreaWidth", 100, m_iniFilePath.c_str());

    GetPrivateProfileStringW(L"Layout_SeekBar", L"TimeLetterSpacing", L"0.0", buf, 32, m_iniFilePath.c_str());
    try { m_seekBarTimeLetterSpacing = std::stof(buf); } catch (...) { m_seekBarTimeLetterSpacing = 0.0f; }

    m_nextBaseRightOffset = GetPrivateProfileIntW(L"Layout_NextTrack", L"BaseRightOffset", 250, m_iniFilePath.c_str());
    m_nextBaseBottomOffset = GetPrivateProfileIntW(L"Layout_NextTrack", L"BaseBottomOffset", 80, m_iniFilePath.c_str());
    m_nextArtOffsetX = GetPrivateProfileIntW(L"Layout_NextTrack", L"ArtOffsetX", 0, m_iniFilePath.c_str());
    m_nextArtOffsetY = GetPrivateProfileIntW(L"Layout_NextTrack", L"ArtOffsetY", 0, m_iniFilePath.c_str());
    m_nextArtSize = GetPrivateProfileIntW(L"Layout_NextTrack", L"ArtSize", 40, m_iniFilePath.c_str());

    GetPrivateProfileStringW(L"Layout_NextTrack", L"BgOpacity", L"0.3", buf, 32, m_iniFilePath.c_str());
    try { m_nextBgOpacity = std::stof(buf); } catch (...) { m_nextBgOpacity = 0.3f; }

    GetPrivateProfileStringW(L"Layout_NextTrack", L"FallbackArtOpacity", L"0.5", buf, 32, m_iniFilePath.c_str());
    try { m_nextFallbackArtOpacity = std::stof(buf); } catch (...) { m_nextFallbackArtOpacity = 0.5f; }

    m_nextLabelOffsetX = GetPrivateProfileIntW(L"Layout_NextTrack", L"LabelOffsetX", 0, m_iniFilePath.c_str());
    m_nextLabelOffsetY = GetPrivateProfileIntW(L"Layout_NextTrack", L"LabelOffsetY", -20, m_iniFilePath.c_str());
    GetPrivateProfileStringW(L"Layout_NextTrack", L"LabelFontSize", L"12.0", buf, 32, m_iniFilePath.c_str());
    try { m_nextLabelFontSize = std::stof(buf); } catch (...) { m_nextLabelFontSize = 12.0f; }

    m_nextTitleOffsetX = GetPrivateProfileIntW(L"Layout_NextTrack", L"TitleOffsetX", 50, m_iniFilePath.c_str());
    m_nextTitleOffsetY = GetPrivateProfileIntW(L"Layout_NextTrack", L"TitleOffsetY", 0, m_iniFilePath.c_str());
    GetPrivateProfileStringW(L"Layout_NextTrack", L"TitleFontSize", L"14.0", buf, 32, m_iniFilePath.c_str());
    try { m_nextTitleFontSize = std::stof(buf); } catch (...) { m_nextTitleFontSize = 14.0f; }

    m_nextArtistOffsetX = GetPrivateProfileIntW(L"Layout_NextTrack", L"ArtistOffsetX", 50, m_iniFilePath.c_str());
    m_nextArtistOffsetY = GetPrivateProfileIntW(L"Layout_NextTrack", L"ArtistOffsetY", 20, m_iniFilePath.c_str());
    GetPrivateProfileStringW(L"Layout_NextTrack", L"ArtistFontSize", L"12.0", buf, 32, m_iniFilePath.c_str());
    try { m_nextArtistFontSize = std::stof(buf); } catch (...) { m_nextArtistFontSize = 12.0f; }

    wchar_t pathBuf[MAX_PATH];
    GetPrivateProfileStringW(L"Playlist", L"DefaultPlaylistPath", L"", pathBuf, MAX_PATH, m_iniFilePath.c_str());
    std::wstring loadedPlaylistPath = pathBuf;
    if (loadedPlaylistPath.empty()) {
        std::wstring exePath = GetExecutablePath();
        size_t pos = exePath.find_last_of(L"\\/");
        if (pos != std::wstring::npos) {
            m_defaultPlaylistPath = exePath.substr(0, pos) + L"\\oztone_playlist.lst";
        } else {
            m_defaultPlaylistPath = L"oztone_playlist.lst";
        }
        WritePrivateProfileStringW(L"Playlist", L"DefaultPlaylistPath", m_defaultPlaylistPath.c_str(), m_iniFilePath.c_str());
    } else {
        m_defaultPlaylistPath = loadedPlaylistPath;
    }
}

void ConfigManager::SaveDefaultSettings() {
    WritePrivateProfileStringW(L"Window", L"ShowTitleBar", L"0", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Window", L"ShowWindowFrame", L"0", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Window", L"ShowTaskbar", L"0", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Window", L"WindowX", std::to_wstring(CW_USEDEFAULT).c_str(), m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Window", L"WindowY", std::to_wstring(CW_USEDEFAULT).c_str(), m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Window", L"WindowWidth", L"1024", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Window", L"WindowHeight", L"512", m_iniFilePath.c_str());

    WritePrivateProfileStringW(L"Layout_AppLogo", L"X", L"16", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Layout_AppLogo", L"Y", L"16", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Layout_AppLogo", L"Width", L"64", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Layout_AppLogo", L"Height", L"64", m_iniFilePath.c_str());

    WritePrivateProfileStringW(L"Layout_NowPlaying", L"BaseX", L"30", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Layout_NowPlaying", L"BaseY", L"350", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Layout_NowPlaying", L"ArtOffsetX", L"0", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Layout_NowPlaying", L"ArtOffsetY", L"0", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Layout_NowPlaying", L"ArtSize", L"120", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Layout_NowPlaying", L"BgOpacity", L"0.3", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Layout_NowPlaying", L"FallbackArtOpacity", L"0.5", m_iniFilePath.c_str());

    WritePrivateProfileStringW(L"Layout_NowPlaying", L"TitleOffsetX", L"140", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Layout_NowPlaying", L"TitleOffsetY", L"10", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Layout_NowPlaying", L"TitleFontSize", L"32.0", m_iniFilePath.c_str());
    
    WritePrivateProfileStringW(L"Layout_NowPlaying", L"ArtistOffsetX", L"140", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Layout_NowPlaying", L"ArtistOffsetY", L"55", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Layout_NowPlaying", L"ArtistFontSize", L"18.0", m_iniFilePath.c_str());

    WritePrivateProfileStringW(L"Layout_SeekBar", L"WidthRatio", L"0.95", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Layout_SeekBar", L"Height", L"3", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Layout_SeekBar", L"BottomOffset", L"50", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Layout_SeekBar", L"BgOpacity", L"0.3", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Layout_SeekBar", L"TimeFontFamily", L"Consolas", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Layout_SeekBar", L"TimeFontSize", L"12.0", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Layout_SeekBar", L"TimeAreaWidth", L"100", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Layout_SeekBar", L"TimeLetterSpacing", L"0.0", m_iniFilePath.c_str());

    WritePrivateProfileStringW(L"Layout_NextTrack", L"BaseRightOffset", L"250", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Layout_NextTrack", L"BaseBottomOffset", L"80", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Layout_NextTrack", L"ArtOffsetX", L"0", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Layout_NextTrack", L"ArtOffsetY", L"0", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Layout_NextTrack", L"ArtSize", L"40", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Layout_NextTrack", L"BgOpacity", L"0.3", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Layout_NextTrack", L"FallbackArtOpacity", L"0.5", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Layout_NextTrack", L"LabelOffsetX", L"0", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Layout_NextTrack", L"LabelOffsetY", L"-20", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Layout_NextTrack", L"LabelFontSize", L"12.0", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Layout_NextTrack", L"TitleOffsetX", L"50", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Layout_NextTrack", L"TitleOffsetY", L"0", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Layout_NextTrack", L"TitleFontSize", L"14.0", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Layout_NextTrack", L"ArtistOffsetX", L"50", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Layout_NextTrack", L"ArtistOffsetY", L"20", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Layout_NextTrack", L"ArtistFontSize", L"12.0", m_iniFilePath.c_str());

    std::wstring exePath = GetExecutablePath();
    std::wstring defPlaylistPath;
    size_t pos = exePath.find_last_of(L"\\/");
    if (pos != std::wstring::npos) {
        defPlaylistPath = exePath.substr(0, pos) + L"\\oztone_playlist.lst";
    } else {
        defPlaylistPath = L"oztone_playlist.lst";
    }
    WritePrivateProfileStringW(L"Playlist", L"DefaultPlaylistPath", defPlaylistPath.c_str(), m_iniFilePath.c_str());
}

void ConfigManager::SaveWindowPosition(int x, int y, int width, int height) {
    m_windowX = x;
    m_windowY = y;
    m_windowWidth = width;
    m_windowHeight = height;

    WritePrivateProfileStringW(L"Window", L"WindowX", std::to_wstring(x).c_str(), m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Window", L"WindowY", std::to_wstring(y).c_str(), m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Window", L"WindowWidth", std::to_wstring(width).c_str(), m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Window", L"WindowHeight", std::to_wstring(height).c_str(), m_iniFilePath.c_str());
}

#include "ConfigManager.h"
#include <vector>

ConfigManager::ConfigManager()
    : m_showTitleBar(false), m_showWindowFrame(false), m_showTaskbar(false),
      m_logoX(16), m_logoY(16), m_logoWidth(64), m_logoHeight(64),
      m_baseX(30), m_baseY(350), m_artOffsetX(0), m_artOffsetY(0), m_artSize(120), m_bgOpacity(0.3f) {
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
}

void ConfigManager::SaveDefaultSettings() {
    WritePrivateProfileStringW(L"Window", L"ShowTitleBar", L"0", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Window", L"ShowWindowFrame", L"0", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Window", L"ShowTaskbar", L"0", m_iniFilePath.c_str());

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
}

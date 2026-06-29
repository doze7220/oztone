#include "ConfigManager.h"
#include <vector>

ConfigManager::ConfigManager()
    : m_showTitleBar(false), m_showWindowFrame(false), m_showTaskbar(false) {
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
}

void ConfigManager::SaveDefaultSettings() {
    WritePrivateProfileStringW(L"Window", L"ShowTitleBar", L"0", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Window", L"ShowWindowFrame", L"0", m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"Window", L"ShowTaskbar", L"0", m_iniFilePath.c_str());
}

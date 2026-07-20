#include "ConfigManager.h"
#include <cassert>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>

#include "ConfigManager_DefaultIni.h"

ConfigManager::ConfigManager() {
  ResetToDefaults();
}

void ConfigManager::ResetToDefaults() {
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

  std::error_code ec;
  m_lastIniWriteTime = std::filesystem::last_write_time(m_iniFilePath, ec);

  return true;
}

bool ConfigManager::CheckForUpdates() {
  if (m_iniFilePath.empty())
    return false;
  std::error_code ec;
  auto currentWriteTime = std::filesystem::last_write_time(m_iniFilePath, ec);
  if (!ec) {
    if (currentWriteTime != m_lastIniWriteTime) {
      m_lastIniWriteTime = currentWriteTime;
      return true;
    }
  }
  return false;
}

std::wstring ConfigManager::GetExecutablePath() const {
  std::vector<wchar_t> buffer(MAX_PATH);
  DWORD length = GetModuleFileNameW(nullptr, buffer.data(),
                                    static_cast<DWORD>(buffer.size()));
  if (length == 0) {
    return L"";
  }
  return std::wstring(buffer.data(), length);
}

std::wstring ConfigManager::GetDefaultIniValue(const std::wstring& section, const std::wstring& key) const {
    std::wistringstream iss(DEFAULT_INI_CONTENT);
    std::wstring line;
    std::wstring targetSection = L"[" + section + L"]";
    std::wstring targetKey = key + L"=";
    bool inSection = false;

    while (std::getline(iss, line)) {
        line.erase(0, line.find_first_not_of(L" \t\r\n"));
        line.erase(line.find_last_not_of(L" \t\r\n") + 1);

        if (line.empty() || line[0] == L';') continue;

        if (line[0] == L'[') {
            inSection = (line == targetSection);
            continue;
        }

        if (inSection && line.find(targetKey) == 0) {
            std::wstring val = line.substr(targetKey.length());
            if (val.size() >= 2 && val.front() == L'"' && val.back() == L'"') {
                val = val.substr(1, val.size() - 2);
            }
            return val;
        }
    }

    assert(false && "Missing key in DEFAULT_INI_CONTENT");
    return L"";
}

int ConfigManager::LoadOrWriteInt(const std::wstring& section, const std::wstring& key) {
    std::wstring defStr = GetDefaultIniValue(section, key);
    int defaultValue = 0;
    try { defaultValue = std::stoi(defStr); } catch (...) {}

    wchar_t buf[32];
    DWORD len = GetPrivateProfileStringW(section.c_str(), key.c_str(), L"__MISSING__", buf, 32, m_iniFilePath.c_str());
    if (len > 0 && wcscmp(buf, L"__MISSING__") == 0) {
        WritePrivateProfileStringW(section.c_str(), key.c_str(), defStr.c_str(), m_iniFilePath.c_str());
        return defaultValue;
    }
    try {
        return std::stoi(buf);
    } catch (...) {
        return defaultValue;
    }
}

float ConfigManager::LoadOrWriteFloat(const std::wstring& section, const std::wstring& key) {
    std::wstring defStr = GetDefaultIniValue(section, key);
    float defaultValue = 0.0f;
    try { defaultValue = std::stof(defStr); } catch (...) {}

    wchar_t buf[32];
    DWORD len = GetPrivateProfileStringW(section.c_str(), key.c_str(), L"__MISSING__", buf, 32, m_iniFilePath.c_str());
    if (len > 0 && wcscmp(buf, L"__MISSING__") == 0) {
        WritePrivateProfileStringW(section.c_str(), key.c_str(), defStr.c_str(), m_iniFilePath.c_str());
        return defaultValue;
    }
    try {
        return std::stof(buf);
    } catch (...) {
        return defaultValue;
    }
}

std::wstring ConfigManager::LoadOrWriteString(const std::wstring& section, const std::wstring& key) {
    std::wstring defaultValue = GetDefaultIniValue(section, key);

    wchar_t buf[256];
    DWORD len = GetPrivateProfileStringW(section.c_str(), key.c_str(), L"__MISSING__", buf, 256, m_iniFilePath.c_str());
    if (len > 0 && wcscmp(buf, L"__MISSING__") == 0) {
        WritePrivateProfileStringW(section.c_str(), key.c_str(), defaultValue.c_str(), m_iniFilePath.c_str());
        return defaultValue;
    }
    return std::wstring(buf);
}

void ConfigManager::LoadSettings() {
  // キャッシュをクリアしてファイルから最新状態を読み込むように強制する
  WritePrivateProfileStringW(NULL, NULL, NULL, m_iniFilePath.c_str());

  LoadCommonSettings();
  LoadWindowSettings();
  LoadBackgroundSettings();
  LoadPlaylistSettings();
  LoadPlaybackSettings();
  LoadLogoMenuSettings();
  LoadVisualizerSettings();
  LoadSystemSettings();
}

void ConfigManager::LoadCommonSettings() {
  m_focusColor = LoadOrWriteString(L"UI_Common_Parm", L"FocusColor");
  m_hoverFadeOutSpeed = LoadOrWriteFloat(L"UI_Common_Parm", L"HoverFadeOutSpeed");
  m_baseLeaveDelay = LoadOrWriteFloat(L"UI_Common_Parm", L"BaseLeaveDelay");
  m_baseFontFamily = LoadOrWriteString(L"UI_Common_Parm", L"BaseFontFamily");
  m_monoFontFamily = LoadOrWriteString(L"UI_Common_Parm", L"MonoFontFamily");
  m_iconFontFamily = LoadOrWriteString(L"UI_Common_Parm", L"IconFontFamily");
  m_osdFontFamily = LoadOrWriteString(L"UI_Common_Parm", L"OsdFontFamily");
  m_enableShadow = LoadOrWriteInt(L"UI_Common_Parm", L"EnableShadow") != 0;
  m_shadowColor = LoadOrWriteString(L"UI_Common_Parm", L"ShadowColor");
  m_shadowOffsetX = LoadOrWriteFloat(L"UI_Common_Parm", L"ShadowOffsetX");
  m_shadowOffsetY = LoadOrWriteFloat(L"UI_Common_Parm", L"ShadowOffsetY");
  m_shadowOpacity = LoadOrWriteFloat(L"UI_Common_Parm", L"ShadowOpacity");
}

void ConfigManager::SaveDefaultSettings() {
  if (std::filesystem::exists(m_iniFilePath)) {
    auto t = std::time(nullptr);
    std::tm tm{};
    localtime_s(&tm, &t);
    std::wostringstream wss;
    wss << m_iniFilePath << L"." << std::put_time(&tm, L"%Y%m%d_%H%M%S")
        << L".bak";
    std::error_code ec;
    std::filesystem::copy_file(
        m_iniFilePath, wss.str(),
        std::filesystem::copy_options::overwrite_existing, ec);
  }

  std::ofstream ofs(m_iniFilePath);
  if (ofs) {
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, DEFAULT_INI_CONTENT, -1, NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, DEFAULT_INI_CONTENT, -1, &strTo[0], size_needed, NULL, NULL);
    ofs << strTo.c_str();
    ofs.close();
  }

  std::wstring exePath = GetExecutablePath();
  std::wstring defPlaylistPath;
  size_t pos = exePath.find_last_of(L"\\/");
  if (pos != std::wstring::npos) {
    defPlaylistPath = exePath.substr(0, pos) + L"\\playlist.ozl";
  } else {
    defPlaylistPath = L"playlist.ozl";
  }
  WritePrivateProfileStringW(L"Playlist", L"DefaultPlaylistPath",
                             defPlaylistPath.c_str(), m_iniFilePath.c_str());
}


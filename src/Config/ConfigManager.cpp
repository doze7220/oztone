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

  LoadSection_System(m_configSystem);
  LoadSection_Window(m_configWindow);
  LoadSection_Visibility(m_configVisibility);
  LoadSection_Playlist(m_configPlaylist);
  LoadSection_Audio(m_configAudio);
  LoadSection_TrackDrum(m_configTrackDrum);
  LoadSection_UICommonParm(m_configUICommonParm);
  LoadSection_Background(m_configBackground);
  LoadSection_LayoutAppLogo(m_configLayoutAppLogo);
  LoadSection_LayoutLogoMenu(m_configLayoutLogoMenu);
  LoadSection_LayoutNowPlaying(m_configLayoutNowPlaying);
  LoadSection_LayoutSeekBar(m_configLayoutSeekBar);
  LoadSection_LayoutPlaybackControls(m_configLayoutPlaybackControls);
  LoadSection_LayoutVolumeControl(m_configLayoutVolumeControl);
  LoadSection_LayoutTooltip(m_configLayoutTooltip);
  LoadSection_LayoutPlaylist(m_configLayoutPlaylist);
  LoadSection_LayoutGlobalHotkeys(m_configLayoutGlobalHotkeys);
  LoadSection_LayoutOSD(m_configLayoutOSD);
  LoadSection_Visualizer(m_configVisualizer);
  LoadSection_VisualizerPrismBeat(m_configVisualizerPrismBeat);
  LoadSection_VisualizerHaloDust(m_configVisualizerHaloDust);
  LoadSection_GlobalHotkeys(m_configGlobalHotkeys);
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

  std::ofstream ofs(m_iniFilePath, std::ios::binary);
  if (ofs) {
    const char bom[] = { '\xFF', '\xFE' };
    ofs.write(bom, sizeof(bom));
    std::wstring content(DEFAULT_INI_CONTENT);
    ofs.write(reinterpret_cast<const char*>(content.c_str()), content.length() * sizeof(wchar_t));
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


void ConfigManager::SaveWindowPosition(int x, int y, int width, int height) {
  m_configWindow.WindowX = x;
  m_configWindow.WindowY = y;
  m_configWindow.WindowWidth = width;
  m_configWindow.WindowHeight = height;
  WritePrivateProfileStringW(L"Window", L"WindowX", std::to_wstring(x).c_str(), m_iniFilePath.c_str());
  WritePrivateProfileStringW(L"Window", L"WindowY", std::to_wstring(y).c_str(), m_iniFilePath.c_str());
  WritePrivateProfileStringW(L"Window", L"WindowWidth", std::to_wstring(width).c_str(), m_iniFilePath.c_str());
  WritePrivateProfileStringW(L"Window", L"WindowHeight", std::to_wstring(height).c_str(), m_iniFilePath.c_str());
}

void ConfigManager::SetZOrder(int zOrder) {
  m_configWindow.ZOrder = zOrder;
  WritePrivateProfileStringW(L"Window", L"ZOrder", std::to_wstring(zOrder).c_str(), m_iniFilePath.c_str());
}

void ConfigManager::SetSavePositionOnExit(bool save) {
  m_configWindow.SavePositionOnExit = save;
  WritePrivateProfileStringW(L"Window", L"SavePositionOnExit", save ? L"1" : L"0", m_iniFilePath.c_str());
}

void ConfigManager::SetEnableResize(bool enable) {
  m_configWindow.EnableResize = enable;
  WritePrivateProfileStringW(L"Window", L"EnableResize", enable ? L"1" : L"0", m_iniFilePath.c_str());
}

void ConfigManager::SetLockWindowPosition(bool lock) {
  m_configWindow.LockWindowPosition = lock;
  WritePrivateProfileStringW(L"Window", L"LockWindowPosition", lock ? L"1" : L"0", m_iniFilePath.c_str());
}

void ConfigManager::SetIsPlaylistPinned(bool pinned) {
  m_configLayoutPlaylist.IsPlaylistPinned = pinned;
  WritePrivateProfileStringW(L"Layout_Playlist", L"IsPlaylistPinned", pinned ? L"1" : L"0", m_iniFilePath.c_str());
}

void ConfigManager::SetShuffleMode(bool mode) {
  m_configAudio.ShuffleMode = mode;
  WritePrivateProfileStringW(L"Audio", L"ShuffleMode", mode ? L"1" : L"0", m_iniFilePath.c_str());
}

void ConfigManager::SetBackgroundArtMode(int mode) {
  m_configBackground.BackgroundArtMode = mode;
  WritePrivateProfileStringW(L"Background", L"BackgroundArtMode", std::to_wstring(mode).c_str(), m_iniFilePath.c_str());
}

void ConfigManager::SetVisualizerMode(int mode) {
  m_configVisualizer.VisualizerMode = mode;
  WritePrivateProfileStringW(L"Visualizer", L"VisualizerMode", std::to_wstring(mode).c_str(), m_iniFilePath.c_str());
}

void ConfigManager::SetShowHotkeys(bool show) {
  m_configGlobalHotkeys.ShowHotkeys = show;
  WritePrivateProfileStringW(L"GlobalHotkeys", L"ShowHotkeys", show ? L"1" : L"0", m_iniFilePath.c_str());
}

void ConfigManager::SetEnableOSD(bool enable) {
  m_configLayoutOSD.EnableOSD = enable;
  WritePrivateProfileStringW(L"Layout_OSD", L"EnableOSD", enable ? L"1" : L"0", m_iniFilePath.c_str());
}

void ConfigManager::SetPlaylistPosition(int position) {
  m_configLayoutPlaylist.PlaylistPosition = position;
  WritePrivateProfileStringW(L"Layout_Playlist", L"PlaylistPosition", std::to_wstring(position).c_str(), m_iniFilePath.c_str());
}

void ConfigManager::SetDefaultVolume(float volume) {
  m_configLayoutVolumeControl.DefaultVolume = volume;
  wchar_t buf[32];
  swprintf_s(buf, L"%.2f", volume);
  WritePrivateProfileStringW(L"Layout_VolumeControl", L"DefaultVolume", buf, m_iniFilePath.c_str());
}

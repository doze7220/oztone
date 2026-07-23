import re
import os

h_file = 'src/ConfigManager.h'
cpp_file = 'src/ConfigManager.cpp'

with open(h_file, 'r', encoding='utf-8') as f:
    h_content = f.read()

# 1. Remove getters and setters (Lines from bool GetShowTitleBar() ... down to void SetExitAppHotkey(int mod, int vk);)
# Actually, I'll use regex to remove sections of lines.
# Delete from `bool GetShowTitleBar() const` up to the line before `bool CheckForUpdates();`
h_content = re.sub(r'\s*bool GetShowTitleBar\(\) const \{ return m_showTitleBar; \}.*?(?=\s*/\*\*\n\s*\*\s*@brief 更新がないか確認する)', '', h_content, flags=re.DOTALL)

# Also delete from `int GetLogoX() const` up to `int GetLogoMenuLockIconOffsetY() const`
h_content = re.sub(r'\s*int GetLogoX\(\) const \{ return m_logoX; \}.*?int GetLogoMenuLockIconOffsetY\(\) const \{ return m_logoMenuLockIconOffsetY; \}', '', h_content, flags=re.DOTALL)

# Delete from `float GetWatchdogInterval() const` up to `std::wstring GetPlaylistGripArrowColor() const`
# wait, actually let's delete all getters up to `void LoadSettings();`
# The block is from `float GetWatchdogInterval() const` down to the line before `void LoadSettings();`
h_content = re.sub(r'\s*float GetWatchdogInterval\(\) const \{ return m_watchdogInterval; \}.*?(?=\s*void LoadSettings\(\);)', '', h_content, flags=re.DOTALL)

# 2. Add back the NEEDED setters into the h_file right before `bool CheckForUpdates();`
needed_setters = """
    void SaveWindowPosition(int x, int y, int width, int height);
    void SetZOrder(int zOrder);
    void SetSavePositionOnExit(bool save);
    void SetEnableResize(bool enable);
    void SetLockWindowPosition(bool lock);
    void SetIsPlaylistPinned(bool pinned);
    void SetShuffleMode(bool mode);
    void SetBackgroundArtMode(int mode);
    void SetVisualizerMode(int mode);
    void SetShowHotkeys(bool show);
    void SetEnableOSD(bool enable);
    void SetPlaylistPosition(int position);
    void SetDefaultVolume(float volume);
"""
h_content = h_content.replace('    bool CheckForUpdates();', needed_setters + '\n    bool CheckForUpdates();')


# 3. Remove old private LoadSettings methods from h_file
# `void LoadCommonSettings();` to `void LoadSystemSettings();`
h_content = re.sub(r'\s*void LoadCommonSettings\(\);.*?void LoadSystemSettings\(\);', '', h_content, flags=re.DOTALL)

# 4. Remove old member variables from h_file
# From `bool m_showTitleBar;` to `float m_thumbnailJpegQuality;` (the end of the class)
h_content = re.sub(r'\s*bool m_showTitleBar;.*?(?=\s*\};)', '', h_content, flags=re.DOTALL)


with open(h_file, 'w', encoding='utf-8') as f:
    f.write(h_content)


with open(cpp_file, 'r', encoding='utf-8') as f:
    cpp_content = f.read()

# Remove the old Load* calls inside LoadSettings()
cpp_content = re.sub(r'\s*LoadCommonSettings\(\);.*?LoadSystemSettings\(\);', '', cpp_content, flags=re.DOTALL)

# Remove the old LoadCommonSettings() implementation
cpp_content = re.sub(r'void ConfigManager::LoadCommonSettings\(\) \{.*?\}\n\n', '', cpp_content, flags=re.DOTALL)

# Add the implementations of the needed setters at the end of cpp_content
needed_implementations = """
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
"""

with open(cpp_file, 'w', encoding='utf-8') as f:
    f.write(cpp_content + needed_implementations)

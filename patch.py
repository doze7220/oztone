import sys

with open('D:/ozlab/oztone/src/ConfigManager.cpp', 'r', encoding='utf-8') as f:
    content = f.read()

defaults = """
  m_showHotkeys = false;
  m_modNextTrack = 10;
  m_vkNextTrack = 39;
  m_modPrevTrack = 10;
  m_vkPrevTrack = 37;
  m_modPlayPause = 10;
  m_vkPlayPause = 38;
  m_modStop = 10;
  m_vkStop = 40;
  m_modVolUp5 = 13;
  m_vkVolUp5 = 39;
  m_modVolDown5 = 13;
  m_vkVolDown5 = 37;
  m_modVolUp25 = 13;
  m_vkVolUp25 = 38;
  m_modVolDown25 = 13;
  m_vkVolDown25 = 40;
  m_modPrevPlaylist = 10;
  m_vkPrevPlaylist = 33;
  m_modNextPlaylist = 10;
  m_vkNextPlaylist = 34;
  m_modActiveTopMost = 10;
  m_vkActiveTopMost = 36;
  m_modActiveBottom = 10;
  m_vkActiveBottom = 35;
  m_modExitApp = 10;
  m_vkExitApp = 46;

  m_ghFontFamily = L"Meiryo";
  m_ghFontSize = 14.0f;
  m_ghLineSpacing = 20.0f;
  m_ghCoreColor = L"#FFFFFF";
  m_ghGlowColor = L"#00FFFF";
  m_ghShadowColor = L"#000000";
  m_ghShadowOpacity = 0.8f;
"""

loads = """
  wchar_t buf[32];
  GetPrivateProfileStringW(L"Layout_GlobalHotkeys", L"FontFamily", L"Meiryo", buf, 32, m_iniFilePath.c_str());
  m_ghFontFamily = buf;
  GetPrivateProfileStringW(L"Layout_GlobalHotkeys", L"FontSize", L"14.0", buf, 32, m_iniFilePath.c_str());
  try { m_ghFontSize = std::stof(buf); } catch (...) { m_ghFontSize = 14.0f; }
  GetPrivateProfileStringW(L"Layout_GlobalHotkeys", L"LineSpacing", L"20.0", buf, 32, m_iniFilePath.c_str());
  try { m_ghLineSpacing = std::stof(buf); } catch (...) { m_ghLineSpacing = 20.0f; }
  GetPrivateProfileStringW(L"Layout_GlobalHotkeys", L"CoreColor", L"#FFFFFF", buf, 32, m_iniFilePath.c_str());
  m_ghCoreColor = buf;
  GetPrivateProfileStringW(L"Layout_GlobalHotkeys", L"GlowColor", L"#00FFFF", buf, 32, m_iniFilePath.c_str());
  m_ghGlowColor = buf;
  GetPrivateProfileStringW(L"Layout_GlobalHotkeys", L"ShadowColor", L"#000000", buf, 32, m_iniFilePath.c_str());
  m_ghShadowColor = buf;
  GetPrivateProfileStringW(L"Layout_GlobalHotkeys", L"ShadowOpacity", L"0.8", buf, 32, m_iniFilePath.c_str());
  try { m_ghShadowOpacity = std::stof(buf); } catch (...) { m_ghShadowOpacity = 0.8f; }

  m_showHotkeys = GetPrivateProfileIntW(L"GlobalHotkeys", L"ShowHotkeys", 0, m_iniFilePath.c_str()) != 0;
  m_modNextTrack = GetPrivateProfileIntW(L"GlobalHotkeys", L"Modifier_NextTrack", 10, m_iniFilePath.c_str());
  m_vkNextTrack = GetPrivateProfileIntW(L"GlobalHotkeys", L"VK_NextTrack", 39, m_iniFilePath.c_str());
  m_modPrevTrack = GetPrivateProfileIntW(L"GlobalHotkeys", L"Modifier_PrevTrack", 10, m_iniFilePath.c_str());
  m_vkPrevTrack = GetPrivateProfileIntW(L"GlobalHotkeys", L"VK_PrevTrack", 37, m_iniFilePath.c_str());
  m_modPlayPause = GetPrivateProfileIntW(L"GlobalHotkeys", L"Modifier_PlayPause", 10, m_iniFilePath.c_str());
  m_vkPlayPause = GetPrivateProfileIntW(L"GlobalHotkeys", L"VK_PlayPause", 38, m_iniFilePath.c_str());
  m_modStop = GetPrivateProfileIntW(L"GlobalHotkeys", L"Modifier_Stop", 10, m_iniFilePath.c_str());
  m_vkStop = GetPrivateProfileIntW(L"GlobalHotkeys", L"VK_Stop", 40, m_iniFilePath.c_str());
  m_modVolUp5 = GetPrivateProfileIntW(L"GlobalHotkeys", L"Modifier_VolUp5", 13, m_iniFilePath.c_str());
  m_vkVolUp5 = GetPrivateProfileIntW(L"GlobalHotkeys", L"VK_VolUp5", 39, m_iniFilePath.c_str());
  m_modVolDown5 = GetPrivateProfileIntW(L"GlobalHotkeys", L"Modifier_VolDown5", 13, m_iniFilePath.c_str());
  m_vkVolDown5 = GetPrivateProfileIntW(L"GlobalHotkeys", L"VK_VolDown5", 37, m_iniFilePath.c_str());
  m_modVolUp25 = GetPrivateProfileIntW(L"GlobalHotkeys", L"Modifier_VolUp25", 13, m_iniFilePath.c_str());
  m_vkVolUp25 = GetPrivateProfileIntW(L"GlobalHotkeys", L"VK_VolUp25", 38, m_iniFilePath.c_str());
  m_modVolDown25 = GetPrivateProfileIntW(L"GlobalHotkeys", L"Modifier_VolDown25", 13, m_iniFilePath.c_str());
  m_vkVolDown25 = GetPrivateProfileIntW(L"GlobalHotkeys", L"VK_VolDown25", 40, m_iniFilePath.c_str());
  m_modPrevPlaylist = GetPrivateProfileIntW(L"GlobalHotkeys", L"Modifier_PrevPlaylist", 10, m_iniFilePath.c_str());
  m_vkPrevPlaylist = GetPrivateProfileIntW(L"GlobalHotkeys", L"VK_PrevPlaylist", 33, m_iniFilePath.c_str());
  m_modNextPlaylist = GetPrivateProfileIntW(L"GlobalHotkeys", L"Modifier_NextPlaylist", 10, m_iniFilePath.c_str());
  m_vkNextPlaylist = GetPrivateProfileIntW(L"GlobalHotkeys", L"VK_NextPlaylist", 34, m_iniFilePath.c_str());
  m_modActiveTopMost = GetPrivateProfileIntW(L"GlobalHotkeys", L"Modifier_ActiveTopMost", 10, m_iniFilePath.c_str());
  m_vkActiveTopMost = GetPrivateProfileIntW(L"GlobalHotkeys", L"VK_ActiveTopMost", 36, m_iniFilePath.c_str());
  m_modActiveBottom = GetPrivateProfileIntW(L"GlobalHotkeys", L"Modifier_ActiveBottom", 10, m_iniFilePath.c_str());
  m_vkActiveBottom = GetPrivateProfileIntW(L"GlobalHotkeys", L"VK_ActiveBottom", 35, m_iniFilePath.c_str());
  m_modExitApp = GetPrivateProfileIntW(L"GlobalHotkeys", L"Modifier_ExitApp", 10, m_iniFilePath.c_str());
  m_vkExitApp = GetPrivateProfileIntW(L"GlobalHotkeys", L"VK_ExitApp", 46, m_iniFilePath.c_str());
"""

setters = """
void ConfigManager::SetShowHotkeys(bool show) {
    m_showHotkeys = show;
    WritePrivateProfileStringW(L"GlobalHotkeys", L"ShowHotkeys", show ? L"1" : L"0", m_iniFilePath.c_str());
}
void ConfigManager::SetNextTrackHotkey(int mod, int vk) {
    m_modNextTrack = mod; m_vkNextTrack = vk;
    WritePrivateProfileStringW(L"GlobalHotkeys", L"Modifier_NextTrack", std::to_wstring(mod).c_str(), m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"GlobalHotkeys", L"VK_NextTrack", std::to_wstring(vk).c_str(), m_iniFilePath.c_str());
}
void ConfigManager::SetPrevTrackHotkey(int mod, int vk) {
    m_modPrevTrack = mod; m_vkPrevTrack = vk;
    WritePrivateProfileStringW(L"GlobalHotkeys", L"Modifier_PrevTrack", std::to_wstring(mod).c_str(), m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"GlobalHotkeys", L"VK_PrevTrack", std::to_wstring(vk).c_str(), m_iniFilePath.c_str());
}
void ConfigManager::SetPlayPauseHotkey(int mod, int vk) {
    m_modPlayPause = mod; m_vkPlayPause = vk;
    WritePrivateProfileStringW(L"GlobalHotkeys", L"Modifier_PlayPause", std::to_wstring(mod).c_str(), m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"GlobalHotkeys", L"VK_PlayPause", std::to_wstring(vk).c_str(), m_iniFilePath.c_str());
}
void ConfigManager::SetStopHotkey(int mod, int vk) {
    m_modStop = mod; m_vkStop = vk;
    WritePrivateProfileStringW(L"GlobalHotkeys", L"Modifier_Stop", std::to_wstring(mod).c_str(), m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"GlobalHotkeys", L"VK_Stop", std::to_wstring(vk).c_str(), m_iniFilePath.c_str());
}
void ConfigManager::SetVolUp5Hotkey(int mod, int vk) {
    m_modVolUp5 = mod; m_vkVolUp5 = vk;
    WritePrivateProfileStringW(L"GlobalHotkeys", L"Modifier_VolUp5", std::to_wstring(mod).c_str(), m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"GlobalHotkeys", L"VK_VolUp5", std::to_wstring(vk).c_str(), m_iniFilePath.c_str());
}
void ConfigManager::SetVolDown5Hotkey(int mod, int vk) {
    m_modVolDown5 = mod; m_vkVolDown5 = vk;
    WritePrivateProfileStringW(L"GlobalHotkeys", L"Modifier_VolDown5", std::to_wstring(mod).c_str(), m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"GlobalHotkeys", L"VK_VolDown5", std::to_wstring(vk).c_str(), m_iniFilePath.c_str());
}
void ConfigManager::SetVolUp25Hotkey(int mod, int vk) {
    m_modVolUp25 = mod; m_vkVolUp25 = vk;
    WritePrivateProfileStringW(L"GlobalHotkeys", L"Modifier_VolUp25", std::to_wstring(mod).c_str(), m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"GlobalHotkeys", L"VK_VolUp25", std::to_wstring(vk).c_str(), m_iniFilePath.c_str());
}
void ConfigManager::SetVolDown25Hotkey(int mod, int vk) {
    m_modVolDown25 = mod; m_vkVolDown25 = vk;
    WritePrivateProfileStringW(L"GlobalHotkeys", L"Modifier_VolDown25", std::to_wstring(mod).c_str(), m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"GlobalHotkeys", L"VK_VolDown25", std::to_wstring(vk).c_str(), m_iniFilePath.c_str());
}
void ConfigManager::SetNextPlaylistHotkey(int mod, int vk) {
    m_modNextPlaylist = mod; m_vkNextPlaylist = vk;
    WritePrivateProfileStringW(L"GlobalHotkeys", L"Modifier_NextPlaylist", std::to_wstring(mod).c_str(), m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"GlobalHotkeys", L"VK_NextPlaylist", std::to_wstring(vk).c_str(), m_iniFilePath.c_str());
}
void ConfigManager::SetPrevPlaylistHotkey(int mod, int vk) {
    m_modPrevPlaylist = mod; m_vkPrevPlaylist = vk;
    WritePrivateProfileStringW(L"GlobalHotkeys", L"Modifier_PrevPlaylist", std::to_wstring(mod).c_str(), m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"GlobalHotkeys", L"VK_PrevPlaylist", std::to_wstring(vk).c_str(), m_iniFilePath.c_str());
}
void ConfigManager::SetActiveTopMostHotkey(int mod, int vk) {
    m_modActiveTopMost = mod; m_vkActiveTopMost = vk;
    WritePrivateProfileStringW(L"GlobalHotkeys", L"Modifier_ActiveTopMost", std::to_wstring(mod).c_str(), m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"GlobalHotkeys", L"VK_ActiveTopMost", std::to_wstring(vk).c_str(), m_iniFilePath.c_str());
}
void ConfigManager::SetActiveBottomHotkey(int mod, int vk) {
    m_modActiveBottom = mod; m_vkActiveBottom = vk;
    WritePrivateProfileStringW(L"GlobalHotkeys", L"Modifier_ActiveBottom", std::to_wstring(mod).c_str(), m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"GlobalHotkeys", L"VK_ActiveBottom", std::to_wstring(vk).c_str(), m_iniFilePath.c_str());
}
void ConfigManager::SetExitAppHotkey(int mod, int vk) {
    m_modExitApp = mod; m_vkExitApp = vk;
    WritePrivateProfileStringW(L"GlobalHotkeys", L"Modifier_ExitApp", std::to_wstring(mod).c_str(), m_iniFilePath.c_str());
    WritePrivateProfileStringW(L"GlobalHotkeys", L"VK_ExitApp", std::to_wstring(vk).c_str(), m_iniFilePath.c_str());
}
"""

if "m_playlistGripShadowOpacity = 0.7f;" in content and "m_showHotkeys = false;" not in content:
    content = content.replace("m_playlistGripShadowOpacity = 0.7f;", "m_playlistGripShadowOpacity = 0.7f;" + defaults)

tgt = "  m_defaultPlaylistPath = L\"\";\n  wchar_t plPathBuf[1024];\n  if (GetPrivateProfileStringW(L\"System\", L\"DefaultPlaylistPath\", L\"\", plPathBuf, 1024, m_iniFilePath.c_str()) > 0) {\n    std::wstring loadedPlaylistPath(plPathBuf);\n    if (!loadedPlaylistPath.empty() && loadedPlaylistPath.find(L\":\\\\\") == std::wstring::npos) {\n      loadedPlaylistPath = basePath + L\"\\\\\" + loadedPlaylistPath;\n    }\n    m_defaultPlaylistPath = loadedPlaylistPath;\n  }"

# Oh wait, my revert wiped `m_defaultPlaylistPath = L"";` etc block from LoadSettings too!
# Let me just find the end of LoadSettings which is:
tgt2 = "    m_pinSubIconFontSize = 10.0f;\n  }\n\n  wchar_t pathBuf[MAX_PATH];\n\n  GetPrivateProfileStringW(L\"Playlist\", L\"DefaultPlaylistPath\", L\"\", pathBuf,\n                           MAX_PATH, m_iniFilePath.c_str());\n  std::wstring loadedPlaylistPath = pathBuf;\n  if (loadedPlaylistPath.empty()) {\n    std::wstring exePath = GetExecutablePath();\n    size_t pos = exePath.find_last_of(L\"\\\\/\");\n    if (pos != std::wstring::npos) {\n      m_defaultPlaylistPath = exePath.substr(0, pos) + L\"\\\\oztone_playlist.lst\";\n    } else {\n      m_defaultPlaylistPath = L\"oztone_playlist.lst\";\n    }\n    WritePrivateProfileStringW(L\"Playlist\", L\"DefaultPlaylistPath\",\n                               m_defaultPlaylistPath.c_str(),\n                               m_iniFilePath.c_str());\n  } else {\n    m_defaultPlaylistPath = loadedPlaylistPath;\n  }"

if tgt2 in content and "m_ghFontFamily =" not in content:
    content = content.replace(tgt2, tgt2 + "\n" + loads)

if "void ConfigManager::SetShowHotkeys" not in content:
    content += "\n" + setters

with open("D:/ozlab/oztone/src/ConfigManager.cpp", "w", encoding="utf-8") as f:
    f.write(content)

print("Patch applied successfully")

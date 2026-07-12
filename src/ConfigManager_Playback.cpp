#include "ConfigManager.h"
#include <string>

void ConfigManager::SetDefaultVolume(float volume) {
  m_defaultVolume = volume;
  WritePrivateProfileStringW(L"Audio", L"DefaultVolume",
                             std::to_wstring(volume).c_str(),
                             m_iniFilePath.c_str());
}

void ConfigManager::SetShuffleMode(bool mode) {
  m_shuffleMode = mode;
  WritePrivateProfileStringW(L"Audio", L"ShuffleMode", mode ? L"1" : L"0",
                             m_iniFilePath.c_str());
}

void ConfigManager::SetSkipSeconds(float seconds) {
  m_skipSeconds = seconds;
  wchar_t buf[32];
  swprintf_s(buf, L"%.1f", seconds);
  WritePrivateProfileStringW(L"Audio", L"SkipSeconds", buf,
                             m_iniFilePath.c_str());
}

void ConfigManager::LoadPlaybackSettings() {
  m_showPlaybackControls = LoadOrWriteInt(L"Visibility", L"ShowPlaybackControls", 1) != 0;
  m_playbackBaseBottomOffset = LoadOrWriteInt(L"Layout_PlaybackControls", L"BaseBottomOffset", 22);
  m_playbackCenterOffsetX = LoadOrWriteInt(L"Layout_PlaybackControls", L"CenterOffsetX", 0);
  m_playbackButtonSpacing = LoadOrWriteInt(L"Layout_PlaybackControls", L"ButtonSpacing", 55);
  m_playbackButtonSize = LoadOrWriteInt(L"Layout_PlaybackControls", L"ButtonSize", 30);
  m_skipIconPoints = LoadOrWriteString(L"Layout_PlaybackControls", L"SkipIconPoints", L"0.1,-0.5,0.65,-0.5,-0.15,0.0,0.65,0.5,0.1,0.5,-0.65,0.0");
  m_skipTextFontSize = LoadOrWriteFloat(L"Layout_PlaybackControls", L"SkipTextFontSize", 15.0f);
  m_skipTextOffsetX = LoadOrWriteFloat(L"Layout_PlaybackControls", L"SkipTextOffsetX", 0.04f);
  m_skipTextOffsetY = LoadOrWriteFloat(L"Layout_PlaybackControls", L"SkipTextOffsetY", -0.05f);
  m_skipTextShadowColor = LoadOrWriteString(L"Layout_PlaybackControls", L"SkipTextShadowColor", L"#444444");
  m_skipTextShadowOpacity = LoadOrWriteFloat(L"Layout_PlaybackControls", L"SkipTextShadowOpacity", 0.40f);
  m_skipTextShadowShift = LoadOrWriteFloat(L"Layout_PlaybackControls", L"SkipTextShadowShift", 1.00f);

  m_showVolumeControl = LoadOrWriteInt(L"Visibility", L"ShowVolumeControl", 1) != 0;
  m_volBaseLeftOffset = LoadOrWriteInt(L"Layout_VolumeControl", L"BaseLeftOffset", 30);
  m_volBaseBottomOffset = LoadOrWriteInt(L"Layout_VolumeControl", L"BaseBottomOffset", 22);
  m_volIconSize = LoadOrWriteInt(L"Layout_VolumeControl", L"IconSize", 30);
  m_volTextOffsetX = LoadOrWriteInt(L"Layout_VolumeControl", L"TextOffsetX", 40);
  m_volTextOffsetY = LoadOrWriteInt(L"Layout_VolumeControl", L"TextOffsetY", -12);
  m_volTextLetterSpacing = LoadOrWriteFloat(L"Layout_VolumeControl", L"TextLetterSpacing", -3.0f);
  m_volFontSize = LoadOrWriteFloat(L"Layout_VolumeControl", L"FontSize", 24.0f);
  m_volFontFamily = LoadOrWriteString(L"Layout_VolumeControl", L"FontFamily", L"Courier New");
  m_volEnableShadow = LoadOrWriteInt(L"Layout_VolumeControl", L"EnableShadow", 1) != 0;
  m_volShadowOffsetX = LoadOrWriteFloat(L"Layout_VolumeControl", L"ShadowOffsetX", 2.0f);
  m_volShadowOffsetY = LoadOrWriteFloat(L"Layout_VolumeControl", L"ShadowOffsetY", 2.0f);
  m_volShadowOpacity = LoadOrWriteFloat(L"Layout_VolumeControl", L"ShadowOpacity", 0.7f);
  m_volTooltipText = LoadOrWriteString(L"Layout_VolumeControl", L"TooltipText", L"🖱️↕");
  m_volTooltipFontSize = LoadOrWriteFloat(L"Layout_VolumeControl", L"TooltipFontSize", 20.0f);
  m_volTooltipFontFamily = LoadOrWriteString(L"Layout_VolumeControl", L"TooltipFontFamily", L"Segoe UI Emoji");
  m_volTooltipOffsetY = LoadOrWriteFloat(L"Layout_VolumeControl", L"TooltipOffsetY", -5.0f);
  m_volTooltipBgColor = LoadOrWriteString(L"Layout_VolumeControl", L"TooltipBgColor", L"#A0000000");
  m_volTooltipBgOpacity = LoadOrWriteFloat(L"Layout_VolumeControl", L"TooltipBgOpacity", 1.0f);
  m_volTooltipTextColor = LoadOrWriteString(L"Layout_VolumeControl", L"TooltipTextColor", L"#FFFFFF");
  m_volTooltipWidth = LoadOrWriteFloat(L"Layout_VolumeControl", L"TooltipWidth", 65.0f);
  m_volTooltipHeight = LoadOrWriteFloat(L"Layout_VolumeControl", L"TooltipHeight", 40.0f);

  m_showSeekBar = LoadOrWriteInt(L"Visibility", L"ShowSeekBar", 1) != 0;
  m_seekBarMargin = LoadOrWriteFloat(L"Layout_SeekBar", L"SeekBarMargin", 20.0f);
  m_seekBarHeight = LoadOrWriteInt(L"Layout_SeekBar", L"Height", 5);
  m_seekBarBottomOffset = LoadOrWriteInt(L"Layout_SeekBar", L"BottomOffset", 25);
  m_seekBarBgOpacity = LoadOrWriteFloat(L"Layout_SeekBar", L"BgOpacity", 0.3f);
  m_seekBarTimeFontFamily = LoadOrWriteString(L"Layout_SeekBar", L"TimeFontFamily", L"Courier New");
  m_seekBarTimeFontSize = LoadOrWriteFloat(L"Layout_SeekBar", L"TimeFontSize", 18.0f);
  m_seekBarTimeAreaWidth = LoadOrWriteInt(L"Layout_SeekBar", L"TimeAreaWidth", 100);
  m_seekBarTimeLetterSpacing = LoadOrWriteFloat(L"Layout_SeekBar", L"TimeLetterSpacing", -3.0f);

  m_defaultVolume = LoadOrWriteFloat(L"Audio", L"DefaultVolume", 1.0f);
  m_shuffleMode = LoadOrWriteInt(L"Audio", L"ShuffleMode", 1) != 0;
  m_skipSeconds = LoadOrWriteFloat(L"Audio", L"SkipSeconds", 10.0f);
}

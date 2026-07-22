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
  m_showPlaybackControls = LoadOrWriteInt(L"Visibility", L"ShowPlaybackControls") != 0;
  m_playbackBaseBottomOffset = LoadOrWriteInt(L"Layout_PlaybackControls", L"BaseBottomOffset");
  m_playbackCenterOffsetX = LoadOrWriteInt(L"Layout_PlaybackControls", L"CenterOffsetX");
  m_playbackButtonSpacing = LoadOrWriteInt(L"Layout_PlaybackControls", L"ButtonSpacing");
  m_playbackButtonSize = LoadOrWriteInt(L"Layout_PlaybackControls", L"ButtonSize");
  m_controlHoverHeight = LoadOrWriteFloat(L"Layout_PlaybackControls", L"ControlHoverHeight");
  m_skipIconPoints = LoadOrWriteString(L"Layout_PlaybackControls", L"SkipIconPoints");
  m_skipTextFontSize = LoadOrWriteFloat(L"Layout_PlaybackControls", L"SkipTextFontSize");
  m_skipTextOffsetX = LoadOrWriteFloat(L"Layout_PlaybackControls", L"SkipTextOffsetX");
  m_skipTextOffsetY = LoadOrWriteFloat(L"Layout_PlaybackControls", L"SkipTextOffsetY");
  m_skipTextShadowShift = LoadOrWriteFloat(L"Layout_PlaybackControls", L"SkipTextShadowShift");

  m_showVolumeControl = LoadOrWriteInt(L"Visibility", L"ShowVolumeControl") != 0;
  m_volBaseLeftOffset = LoadOrWriteInt(L"Layout_VolumeControl", L"BaseLeftOffset");
  m_volBaseBottomOffset = LoadOrWriteInt(L"Layout_VolumeControl", L"BaseBottomOffset");
  m_volIconSize = LoadOrWriteInt(L"Layout_VolumeControl", L"IconSize");
  m_volTextOffsetX = LoadOrWriteInt(L"Layout_VolumeControl", L"TextOffsetX");
  m_volTextOffsetY = LoadOrWriteInt(L"Layout_VolumeControl", L"TextOffsetY");
  m_volTextLetterSpacing = LoadOrWriteFloat(L"Layout_VolumeControl", L"TextLetterSpacing");
  m_volFontSize = LoadOrWriteFloat(L"Layout_VolumeControl", L"FontSize");
  m_volTooltipOffsetY = LoadOrWriteFloat(L"Layout_VolumeControl", L"TooltipOffsetY");

  m_showSeekBar = LoadOrWriteInt(L"Visibility", L"ShowSeekBar") != 0;
  m_seekBarMargin = LoadOrWriteFloat(L"Layout_SeekBar", L"SeekBarMargin");
  m_seekBarHeight = LoadOrWriteInt(L"Layout_SeekBar", L"Height");
  m_seekBarBottomOffset = LoadOrWriteInt(L"Layout_SeekBar", L"BottomOffset");
  m_seekBarFgColor = LoadOrWriteString(L"Layout_SeekBar", L"FgColor");
  m_seekBarBgColor = LoadOrWriteString(L"Layout_SeekBar", L"BgColor");
  m_seekBarFgOpacity = LoadOrWriteFloat(L"Layout_SeekBar", L"FgOpacity");
  m_seekBarBgOpacity = LoadOrWriteFloat(L"Layout_SeekBar", L"BgOpacity");
  m_seekBarTimeTextColor = LoadOrWriteString(L"Layout_SeekBar", L"TextColor");
  m_seekBarTimeTextOpacity = LoadOrWriteFloat(L"Layout_SeekBar", L"TextOpacity");
  m_seekBarTimeFontSize = LoadOrWriteFloat(L"Layout_SeekBar", L"TimeFontSize");
  m_seekBarTimeMarginRight = LoadOrWriteFloat(L"Layout_SeekBar", L"TimeMarginRight");
  m_seekBarTimeLetterSpacing = LoadOrWriteFloat(L"Layout_SeekBar", L"TimeLetterSpacing");

  m_defaultVolume = LoadOrWriteFloat(L"Audio", L"DefaultVolume");
  m_shuffleMode = LoadOrWriteInt(L"Audio", L"ShuffleMode") != 0;
  m_skipSeconds = LoadOrWriteFloat(L"Audio", L"SkipSeconds");

  m_showNowPlaying = LoadOrWriteInt(L"Visibility", L"ShowNowPlaying") != 0;

  m_baseX = LoadOrWriteInt(L"Layout_NowPlaying", L"BaseX");
  m_baseBottomOffset = LoadOrWriteInt(L"Layout_NowPlaying", L"BaseBottomOffset");
  m_artOffsetX = LoadOrWriteInt(L"Layout_NowPlaying", L"ArtOffsetX");
  m_artOffsetY = LoadOrWriteInt(L"Layout_NowPlaying", L"ArtOffsetY");
  m_artSize = LoadOrWriteInt(L"Layout_NowPlaying", L"ArtSize");
  m_fallbackArtOpacity = LoadOrWriteFloat(L"Layout_NowPlaying", L"FallbackArtOpacity");
  m_titleOffsetX = LoadOrWriteInt(L"Layout_NowPlaying", L"TitleOffsetX");
  m_titleOffsetY = LoadOrWriteInt(L"Layout_NowPlaying", L"TitleOffsetY");
  m_titleFontSize = LoadOrWriteFloat(L"Layout_NowPlaying", L"TitleFontSize");
  m_artistOffsetX = LoadOrWriteInt(L"Layout_NowPlaying", L"ArtistOffsetX");
  m_artistOffsetY = LoadOrWriteInt(L"Layout_NowPlaying", L"ArtistOffsetY");
  m_artistFontSize = LoadOrWriteFloat(L"Layout_NowPlaying", L"ArtistFontSize");
  m_tooltipOffsetX = LoadOrWriteFloat(L"Layout_NowPlaying", L"TooltipOffsetX");
  m_tooltipOffsetY = LoadOrWriteFloat(L"Layout_NowPlaying", L"TooltipOffsetY");


  m_tdEnable = LoadOrWriteInt(L"TrackDrum", L"EnableTrackDrum") != 0;
  m_tdMaxDuration = LoadOrWriteFloat(L"TrackDrum", L"MaxDuration");
  m_tdMinSpeed = LoadOrWriteFloat(L"TrackDrum", L"MinSpeed");
  m_tdMaxSpeed = LoadOrWriteFloat(L"TrackDrum", L"MaxSpeed");
  m_tdAcceleration = LoadOrWriteFloat(L"TrackDrum", L"Acceleration");
  m_tdDeceleration = LoadOrWriteFloat(L"TrackDrum", L"Deceleration");
}

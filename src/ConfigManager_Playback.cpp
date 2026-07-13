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
  m_skipIconPoints = LoadOrWriteString(L"Layout_PlaybackControls", L"SkipIconPoints");
  m_skipTextFontSize = LoadOrWriteFloat(L"Layout_PlaybackControls", L"SkipTextFontSize");
  m_skipTextOffsetX = LoadOrWriteFloat(L"Layout_PlaybackControls", L"SkipTextOffsetX");
  m_skipTextOffsetY = LoadOrWriteFloat(L"Layout_PlaybackControls", L"SkipTextOffsetY");
  m_skipTextShadowColor = LoadOrWriteString(L"Layout_PlaybackControls", L"SkipTextShadowColor");
  m_skipTextShadowOpacity = LoadOrWriteFloat(L"Layout_PlaybackControls", L"SkipTextShadowOpacity");
  m_skipTextShadowShift = LoadOrWriteFloat(L"Layout_PlaybackControls", L"SkipTextShadowShift");

  m_showVolumeControl = LoadOrWriteInt(L"Visibility", L"ShowVolumeControl") != 0;
  m_volBaseLeftOffset = LoadOrWriteInt(L"Layout_VolumeControl", L"BaseLeftOffset");
  m_volBaseBottomOffset = LoadOrWriteInt(L"Layout_VolumeControl", L"BaseBottomOffset");
  m_volIconSize = LoadOrWriteInt(L"Layout_VolumeControl", L"IconSize");
  m_volTextOffsetX = LoadOrWriteInt(L"Layout_VolumeControl", L"TextOffsetX");
  m_volTextOffsetY = LoadOrWriteInt(L"Layout_VolumeControl", L"TextOffsetY");
  m_volTextLetterSpacing = LoadOrWriteFloat(L"Layout_VolumeControl", L"TextLetterSpacing");
  m_volFontSize = LoadOrWriteFloat(L"Layout_VolumeControl", L"FontSize");
  m_volFontFamily = LoadOrWriteString(L"Layout_VolumeControl", L"FontFamily");
  m_volEnableShadow = LoadOrWriteInt(L"Layout_VolumeControl", L"EnableShadow") != 0;
  m_volShadowOffsetX = LoadOrWriteFloat(L"Layout_VolumeControl", L"ShadowOffsetX");
  m_volShadowOffsetY = LoadOrWriteFloat(L"Layout_VolumeControl", L"ShadowOffsetY");
  m_volShadowOpacity = LoadOrWriteFloat(L"Layout_VolumeControl", L"ShadowOpacity");
  m_volTooltipIconSize = LoadOrWriteFloat(L"Layout_VolumeControl", L"TooltipIconSize");
  m_volTooltipOffsetY = LoadOrWriteFloat(L"Layout_VolumeControl", L"TooltipOffsetY");
  m_volTooltipBgColor = LoadOrWriteString(L"Layout_VolumeControl", L"TooltipBgColor");
  m_volTooltipBgOpacity = LoadOrWriteFloat(L"Layout_VolumeControl", L"TooltipBgOpacity");
  m_volTooltipWidth = LoadOrWriteFloat(L"Layout_VolumeControl", L"TooltipWidth");
  m_volTooltipHeight = LoadOrWriteFloat(L"Layout_VolumeControl", L"TooltipHeight");

  m_showSeekBar = LoadOrWriteInt(L"Visibility", L"ShowSeekBar") != 0;
  m_seekBarMargin = LoadOrWriteFloat(L"Layout_SeekBar", L"SeekBarMargin");
  m_seekBarHeight = LoadOrWriteInt(L"Layout_SeekBar", L"Height");
  m_seekBarBottomOffset = LoadOrWriteInt(L"Layout_SeekBar", L"BottomOffset");
  m_seekBarBgOpacity = LoadOrWriteFloat(L"Layout_SeekBar", L"BgOpacity");
  m_seekBarTimeFontFamily = LoadOrWriteString(L"Layout_SeekBar", L"TimeFontFamily");
  m_seekBarTimeFontSize = LoadOrWriteFloat(L"Layout_SeekBar", L"TimeFontSize");
  m_seekBarTimeAreaWidth = LoadOrWriteInt(L"Layout_SeekBar", L"TimeAreaWidth");
  m_seekBarTimeLetterSpacing = LoadOrWriteFloat(L"Layout_SeekBar", L"TimeLetterSpacing");

  m_defaultVolume = LoadOrWriteFloat(L"Audio", L"DefaultVolume");
  m_shuffleMode = LoadOrWriteInt(L"Audio", L"ShuffleMode") != 0;
  m_skipSeconds = LoadOrWriteFloat(L"Audio", L"SkipSeconds");

  m_showNowPlaying = LoadOrWriteInt(L"Visibility", L"ShowNowPlaying") != 0;
  m_showNextTrack = LoadOrWriteInt(L"Visibility", L"ShowNextTrack") != 0;

  m_baseX = LoadOrWriteInt(L"Layout_NowPlaying", L"BaseX");
  m_baseBottomOffset = LoadOrWriteInt(L"Layout_NowPlaying", L"BaseBottomOffset");
  m_artOffsetX = LoadOrWriteInt(L"Layout_NowPlaying", L"ArtOffsetX");
  m_artOffsetY = LoadOrWriteInt(L"Layout_NowPlaying", L"ArtOffsetY");
  m_artSize = LoadOrWriteInt(L"Layout_NowPlaying", L"ArtSize");
  m_fallbackArtOpacity = LoadOrWriteFloat(L"Layout_NowPlaying", L"FallbackArtOpacity");
  m_titleOffsetX = LoadOrWriteInt(L"Layout_NowPlaying", L"TitleOffsetX");
  m_titleOffsetY = LoadOrWriteInt(L"Layout_NowPlaying", L"TitleOffsetY");
  m_titleFontSize = LoadOrWriteFloat(L"Layout_NowPlaying", L"TitleFontSize");
  m_titleFontFamily = LoadOrWriteString(L"Layout_NowPlaying", L"TitleFontFamily");
  m_artistOffsetX = LoadOrWriteInt(L"Layout_NowPlaying", L"ArtistOffsetX");
  m_artistOffsetY = LoadOrWriteInt(L"Layout_NowPlaying", L"ArtistOffsetY");
  m_artistFontSize = LoadOrWriteFloat(L"Layout_NowPlaying", L"ArtistFontSize");
  m_artistFontFamily = LoadOrWriteString(L"Layout_NowPlaying", L"ArtistFontFamily");
  m_trackCountFontFamily = LoadOrWriteString(L"Layout_NowPlaying", L"TrackCountFontFamily");
  m_trackCountFontSize = LoadOrWriteFloat(L"Layout_NowPlaying", L"TrackCountFontSize");
  m_trackCountOffsetX = LoadOrWriteInt(L"Layout_NowPlaying", L"TrackCountOffsetX");
  m_trackCountOffsetY = LoadOrWriteInt(L"Layout_NowPlaying", L"TrackCountOffsetY");
  m_trackCountTextAlignment = LoadOrWriteInt(L"Layout_NowPlaying", L"TrackCountTextAlignment");
  m_trackCountLetterSpacing = LoadOrWriteFloat(L"Layout_NowPlaying", L"TrackCountLetterSpacing");
  m_trackCountShadowOffsetX = LoadOrWriteFloat(L"Layout_NowPlaying", L"TrackCountShadowOffsetX");
  m_trackCountShadowOffsetY = LoadOrWriteFloat(L"Layout_NowPlaying", L"TrackCountShadowOffsetY");
  m_trackCountShadowOpacity = LoadOrWriteFloat(L"Layout_NowPlaying", L"TrackCountShadowOpacity");

  m_enableNextTrack = LoadOrWriteInt(L"Layout_NextTrack", L"EnableNextTrack") != 0;
  m_nextBaseRightOffset = LoadOrWriteInt(L"Layout_NextTrack", L"BaseRightOffset");
  m_nextBaseBottomOffset = LoadOrWriteInt(L"Layout_NextTrack", L"BaseBottomOffset");
  m_nextArtOffsetX = LoadOrWriteInt(L"Layout_NextTrack", L"ArtOffsetX");
  m_nextArtOffsetY = LoadOrWriteInt(L"Layout_NextTrack", L"ArtOffsetY");
  m_nextArtSize = LoadOrWriteInt(L"Layout_NextTrack", L"ArtSize");
  m_nextBgOpacity = LoadOrWriteFloat(L"Layout_NextTrack", L"BgOpacity");
  m_nextFallbackArtOpacity = LoadOrWriteFloat(L"Layout_NextTrack", L"FallbackArtOpacity");
  m_nextLabelOffsetX = LoadOrWriteInt(L"Layout_NextTrack", L"LabelOffsetX");
  m_nextLabelOffsetY = LoadOrWriteInt(L"Layout_NextTrack", L"LabelOffsetY");
  m_nextLabelFontSize = LoadOrWriteFloat(L"Layout_NextTrack", L"LabelFontSize");
  m_nextLabelFontFamily = LoadOrWriteString(L"Layout_NextTrack", L"LabelFontFamily");
  m_nextTitleOffsetX = LoadOrWriteInt(L"Layout_NextTrack", L"TitleOffsetX");
  m_nextTitleOffsetY = LoadOrWriteInt(L"Layout_NextTrack", L"TitleOffsetY");
  m_nextTitleFontSize = LoadOrWriteFloat(L"Layout_NextTrack", L"TitleFontSize");
  m_nextTitleFontFamily = LoadOrWriteString(L"Layout_NextTrack", L"TitleFontFamily");
  m_nextArtistOffsetX = LoadOrWriteInt(L"Layout_NextTrack", L"ArtistOffsetX");
  m_nextArtistOffsetY = LoadOrWriteInt(L"Layout_NextTrack", L"ArtistOffsetY");
  m_nextArtistFontSize = LoadOrWriteFloat(L"Layout_NextTrack", L"ArtistFontSize");
  m_nextArtistFontFamily = LoadOrWriteString(L"Layout_NextTrack", L"ArtistFontFamily");
}

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
  m_volTooltipIconSize = LoadOrWriteFloat(L"Layout_VolumeControl", L"TooltipIconSize", 24.0f);
  m_volTooltipOffsetY = LoadOrWriteFloat(L"Layout_VolumeControl", L"TooltipOffsetY", -5.0f);
  m_volTooltipBgColor = LoadOrWriteString(L"Layout_VolumeControl", L"TooltipBgColor", L"#A0000000");
  m_volTooltipBgOpacity = LoadOrWriteFloat(L"Layout_VolumeControl", L"TooltipBgOpacity", 1.0f);
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

  m_showNowPlaying = LoadOrWriteInt(L"Visibility", L"ShowNowPlaying", 1) != 0;
  m_showNextTrack = LoadOrWriteInt(L"Visibility", L"ShowNextTrack", 1) != 0;

  m_baseX = LoadOrWriteInt(L"Layout_NowPlaying", L"BaseX", 30);
  m_baseBottomOffset = LoadOrWriteInt(L"Layout_NowPlaying", L"BaseBottomOffset", 162);
  m_artOffsetX = LoadOrWriteInt(L"Layout_NowPlaying", L"ArtOffsetX", 0);
  m_artOffsetY = LoadOrWriteInt(L"Layout_NowPlaying", L"ArtOffsetY", 0);
  m_artSize = LoadOrWriteInt(L"Layout_NowPlaying", L"ArtSize", 120);
  m_fallbackArtOpacity = LoadOrWriteFloat(L"Layout_NowPlaying", L"FallbackArtOpacity", 0.5f);
  m_titleOffsetX = LoadOrWriteInt(L"Layout_NowPlaying", L"TitleOffsetX", 140);
  m_titleOffsetY = LoadOrWriteInt(L"Layout_NowPlaying", L"TitleOffsetY", 10);
  m_titleFontSize = LoadOrWriteFloat(L"Layout_NowPlaying", L"TitleFontSize", 32.0f);
  m_titleFontFamily = LoadOrWriteString(L"Layout_NowPlaying", L"TitleFontFamily", L"Meiryo");
  m_artistOffsetX = LoadOrWriteInt(L"Layout_NowPlaying", L"ArtistOffsetX", 140);
  m_artistOffsetY = LoadOrWriteInt(L"Layout_NowPlaying", L"ArtistOffsetY", 55);
  m_artistFontSize = LoadOrWriteFloat(L"Layout_NowPlaying", L"ArtistFontSize", 18.0f);
  m_artistFontFamily = LoadOrWriteString(L"Layout_NowPlaying", L"ArtistFontFamily", L"Meiryo");
  m_trackCountFontFamily = LoadOrWriteString(L"Layout_NowPlaying", L"TrackCountFontFamily", L"Courier New");
  m_trackCountFontSize = LoadOrWriteFloat(L"Layout_NowPlaying", L"TrackCountFontSize", 14.0f);
  m_trackCountOffsetX = LoadOrWriteInt(L"Layout_NowPlaying", L"TrackCountOffsetX", 20);
  m_trackCountOffsetY = LoadOrWriteInt(L"Layout_NowPlaying", L"TrackCountOffsetY", 100);
  m_trackCountTextAlignment = LoadOrWriteInt(L"Layout_NowPlaying", L"TrackCountTextAlignment", 0);
  m_trackCountLetterSpacing = LoadOrWriteFloat(L"Layout_NowPlaying", L"TrackCountLetterSpacing", -1.0f);
  m_trackCountShadowOffsetX = LoadOrWriteFloat(L"Layout_NowPlaying", L"TrackCountShadowOffsetX", 2.0f);
  m_trackCountShadowOffsetY = LoadOrWriteFloat(L"Layout_NowPlaying", L"TrackCountShadowOffsetY", 2.0f);
  m_trackCountShadowOpacity = LoadOrWriteFloat(L"Layout_NowPlaying", L"TrackCountShadowOpacity", 0.7f);

  m_enableNextTrack = LoadOrWriteInt(L"Layout_NextTrack", L"EnableNextTrack", 0) != 0;
  m_nextBaseRightOffset = LoadOrWriteInt(L"Layout_NextTrack", L"BaseRightOffset", 250);
  m_nextBaseBottomOffset = LoadOrWriteInt(L"Layout_NextTrack", L"BaseBottomOffset", 80);
  m_nextArtOffsetX = LoadOrWriteInt(L"Layout_NextTrack", L"ArtOffsetX", 0);
  m_nextArtOffsetY = LoadOrWriteInt(L"Layout_NextTrack", L"ArtOffsetY", 0);
  m_nextArtSize = LoadOrWriteInt(L"Layout_NextTrack", L"ArtSize", 40);
  m_nextBgOpacity = LoadOrWriteFloat(L"Layout_NextTrack", L"BgOpacity", 0.3f);
  m_nextFallbackArtOpacity = LoadOrWriteFloat(L"Layout_NextTrack", L"FallbackArtOpacity", 0.5f);
  m_nextLabelOffsetX = LoadOrWriteInt(L"Layout_NextTrack", L"LabelOffsetX", 0);
  m_nextLabelOffsetY = LoadOrWriteInt(L"Layout_NextTrack", L"LabelOffsetY", -20);
  m_nextLabelFontSize = LoadOrWriteFloat(L"Layout_NextTrack", L"LabelFontSize", 12.0f);
  m_nextLabelFontFamily = LoadOrWriteString(L"Layout_NextTrack", L"LabelFontFamily", L"Meiryo");
  m_nextTitleOffsetX = LoadOrWriteInt(L"Layout_NextTrack", L"TitleOffsetX", 50);
  m_nextTitleOffsetY = LoadOrWriteInt(L"Layout_NextTrack", L"TitleOffsetY", 0);
  m_nextTitleFontSize = LoadOrWriteFloat(L"Layout_NextTrack", L"TitleFontSize", 14.0f);
  m_nextTitleFontFamily = LoadOrWriteString(L"Layout_NextTrack", L"TitleFontFamily", L"Meiryo");
  m_nextArtistOffsetX = LoadOrWriteInt(L"Layout_NextTrack", L"ArtistOffsetX", 50);
  m_nextArtistOffsetY = LoadOrWriteInt(L"Layout_NextTrack", L"ArtistOffsetY", 20);
  m_nextArtistFontSize = LoadOrWriteFloat(L"Layout_NextTrack", L"ArtistFontSize", 12.0f);
  m_nextArtistFontFamily = LoadOrWriteString(L"Layout_NextTrack", L"ArtistFontFamily", L"Meiryo");
}

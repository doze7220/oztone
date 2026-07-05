#include "ConfigManager.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <vector>

constexpr const char *DEFAULT_INI_CONTENT = R"(; OZtone Default Configuration

[Playlist]
DefaultPlaylistPath=playlist.ozl

[Window]
ShowTitleBar=0
ShowWindowFrame=0
ShowTaskbar=0
ZOrder=0
SavePositionOnExit=1
EnableResize=0
LockWindowPosition=0
WindowX=-2147483648
WindowY=-2147483648
WindowWidth=1024
WindowHeight=512

[Visibility]
ShowAppLogo=1
ShowNowPlaying=1
ShowNextTrack=1
ShowSeekBar=1
ShowPlaybackControls=1
ShowVolumeControl=1

[Layout_Window]
EnableShadow=1
ShadowOffsetX=2.0
ShadowOffsetY=2.0
ShadowOpacity=0.7
BgOpacity=0.8
BgDarkenOpacity=0.4
BackgroundArtMode=0
ControlHoverHeight=50.0

[Layout_AppLogo]
X=16
Y=16
Width=64
Height=64

[Layout_LogoMenu]
MenuIconSize=44.0
MenuIconSpacing=54
MenuIconOffsetX=-24
MenuIconOffsetY=38
MenuScrollDuration=0.5
MenuFontFamily="Segoe UI Emoji"
MenuTextColor=#FFFFFF
MenuTypingFontFamily=Meiryo
MenuTypingFontSize=14.0
MenuTextOffsetX=8
MenuTextOffsetY=-18
MenuTypingLetterSpacing=-1.0
MenuStrikeLength=48.0
MenuStrikeThickness=4.0
IconHoverBgAlpha=0.4
VisualizerIconFontSize=24.0
VisualizerIconOffsetX=6
VisualizerIconOffsetY=0
DescShadowOffsetX=1.0
DescShadowOffsetY=1.0
DescShadowOpacity=0.8

[Layout_NowPlaying]
BaseX=20
BaseBottomOffset=162
ArtOffsetX=0
ArtOffsetY=0
ArtSize=120
FallbackArtOpacity=0.5
TitleOffsetX=128
TitleOffsetY=81
TitleFontSize=36.0
TitleFontFamily=Meiryo
ArtistOffsetX=130
ArtistOffsetY=60
ArtistFontSize=18.0
ArtistFontFamily=Meiryo
TrackCountOffsetX=-82
TrackCountOffsetY=94
TrackCountFontFamily="Impact"
TrackCountFontSize=22.0
TrackCountLetterSpacing=-1.0
TrackCountShadowOffsetX=2.0
TrackCountShadowOffsetY=2.0
TrackCountShadowOpacity=0.7

[Layout_SeekBar]
SeekBarMargin=20.0
Height=5
BottomOffset=25
BgOpacity=0.3
TimeFontFamily="Courier New"
TimeFontSize=18.0
TimeAreaWidth=100
TimeLetterSpacing=-3.0

[Layout_NextTrack]
EnableNextTrack=0
BaseRightOffset=300
BaseBottomOffset=80
ArtOffsetX=0
ArtOffsetY=0
ArtSize=40
BgOpacity=0.3
FallbackArtOpacity=0.5
LabelOffsetX=0
LabelOffsetY=-20
LabelFontSize=12.0
LabelFontFamily=Meiryo
TitleOffsetX=45
TitleOffsetY=18
TitleFontSize=20.0
TitleFontFamily=Meiryo
ArtistOffsetX=47
ArtistOffsetY=4
ArtistFontSize=12.0
ArtistFontFamily=Meiryo

[Layout_PlaybackControls]
BaseBottomOffset=22
CenterOffsetX=0
ButtonSpacing=55
ButtonSize=30

[Layout_VolumeControl]
BaseLeftOffset=30
BaseBottomOffset=22
IconSize=30
TextOffsetX=40
TextOffsetY=-12
FontSize=24.0
TextLetterSpacing=-3.0
FontFamily="Courier New"
EnableShadow=1
ShadowOffsetX=2.0
ShadowOffsetY=2.0
ShadowOpacity=0.7

[Audio]
DefaultVolume=1.0
ShuffleMode=1

[Visualizer]
VisualizerMode=2

[Layout_Playlist]
PlaylistPosition=1
PlaylistHoverWidth=120
PlaylistWidth=250
PlaylistItemOffsetY=45
PlaylistTitleFontSize=16.0
PlaylistTitleFontFamily=Meiryo
PlaylistTitleOffsetX=20
PlaylistTitleOffsetY=4
PlaylistArtistFontSize=12.0
PlaylistArtistColor=#888888
PlaylistArtistFontFamily=Meiryo
PlaylistArtistOffsetX=20
PlaylistArtistOffsetY=25
PlaylistTimeFontSize=12.0
PlaylistTimeColor=#888888
PlaylistTimeFontFamily=Meiryo
PlaylistTimeOffsetX=10
PlaylistTimeOffsetY=25
PlaylistTimeLetterSpacing=0.0
PlaylistBgOpacity=0.8
PlaylistGripOffset=5.0
PlaylistGripLineWidth=1.0
PlaylistGripLineColor=#AAAAAA
PlaylistGripArrowHeight=35.0
PlaylistGripArrowWidth=15.0
PlaylistGripArrowColor=#AAAAAA
PlaylistGripShadowOffsetX=2.0
PlaylistGripShadowOffsetY=2.0
PlaylistGripShadowOpacity=0.7
ToolbarHeight=60.0
ToolbarIconSize=18.0
ToolbarIconSpacing=10.0
ToolbarTextOffsetY=30.0
ToolbarTextFontSize=12.0

)";

ConfigManager::ConfigManager()
    : m_showTitleBar(false), m_showWindowFrame(false), m_showTaskbar(false),
      m_showAppLogo(true), m_showNowPlaying(true), m_showNextTrack(true),
      m_enableNextTrack(false), m_showSeekBar(true),
      m_showPlaybackControls(true), m_showVolumeControl(true), m_zOrder(0),
      m_savePositionOnExit(true), m_enableResize(false), m_lockWindowPosition(false), m_shuffleMode(true),
      m_windowX(CW_USEDEFAULT), m_windowY(CW_USEDEFAULT), m_windowWidth(1024),
      m_windowHeight(512), m_enableShadow(true), m_shadowOffsetX(2.0f),
      m_shadowOffsetY(2.0f), m_shadowOpacity(0.7f), m_bgDarkenOpacity(0.3f),
      m_bgOpacity(0.8f), m_backgroundArtMode(0), m_visualizerMode(0),
      m_logoX(16), m_logoY(16), m_logoWidth(64), m_logoHeight(64),
      m_logoMenuIconSize(24.0f), m_logoMenuIconSpacing(40),
      m_logoMenuIconOffsetX(0), m_logoMenuIconOffsetY(0),
      m_logoMenuScrollDuration(0.5f), m_logoMenuFontFamily(L"Segoe UI Emoji"),
      m_logoMenuTextColor(L"#FFFFFF"), m_logoMenuTypingFontFamily(L"Consolas"),
      m_logoMenuTypingFontSize(14.0f), m_logoMenuTextOffsetX(0),
      m_logoMenuTextOffsetY(60), m_logoMenuTypingLetterSpacing(0.0f),
      m_logoMenuStrikeLength(20.0f), m_logoMenuStrikeThickness(2.0f),
      m_baseX(30), m_baseBottomOffset(162), m_artOffsetX(0), m_artOffsetY(0),
      m_artSize(120), m_fallbackArtOpacity(0.5f),

      m_titleOffsetX(140), m_titleOffsetY(10), m_titleFontSize(32.0f),
      m_titleFontFamily(L"Meiryo"), m_artistOffsetX(140), m_artistOffsetY(55),
      m_artistFontSize(18.0f), m_artistFontFamily(L"Meiryo"),
      m_seekBarMargin(20.0f), m_seekBarHeight(3), m_seekBarBottomOffset(50),
      m_seekBarBgOpacity(0.3f), m_seekBarTimeFontFamily(L"Consolas"),
      m_seekBarTimeFontSize(12.0f), m_seekBarTimeAreaWidth(100),
      m_seekBarTimeLetterSpacing(0.0f), m_nextBaseRightOffset(250),
      m_nextBaseBottomOffset(80), m_nextArtOffsetX(0), m_nextArtOffsetY(0),
      m_nextArtSize(40), m_nextBgOpacity(0.3f), m_nextFallbackArtOpacity(0.5f),
      m_nextLabelOffsetX(0), m_nextLabelOffsetY(-20),
      m_nextLabelFontSize(12.0f), m_nextLabelFontFamily(L"Meiryo"),

      m_nextTitleOffsetX(50), m_nextTitleOffsetY(0), m_nextTitleFontSize(14.0f),
      m_nextTitleFontFamily(L"Meiryo"), m_nextArtistOffsetX(50),
      m_nextArtistOffsetY(20), m_nextArtistFontSize(12.0f),
      m_nextArtistFontFamily(L"Meiryo"), m_playbackBaseBottomOffset(40),
      m_playbackCenterOffsetX(0), m_playbackButtonSpacing(40),
      m_playbackButtonSize(20) {
  m_defaultVolume = 1.0f;
  m_controlHoverHeight = 50.0f;
  m_volBaseLeftOffset = 30;
  m_volBaseBottomOffset = 22;
  m_volIconSize = 16;
  m_volTextOffsetX = 31;
  m_volTextOffsetY = -12;
  m_volTextLetterSpacing = 0.0f;
  m_volFontSize = 14.0f;
  m_volFontFamily = L"Meiryo";
  m_volEnableShadow = true;
  m_volShadowOffsetX = 2.0f;
  m_volShadowOffsetY = 2.0f;
  m_volShadowOpacity = 0.7f;

  m_trackCountFontFamily = L"Courier New";
  m_trackCountFontSize = 14.0f;
  m_trackCountOffsetX = 20;
  m_trackCountOffsetY = 100;
  m_trackCountTextAlignment = 1;
  m_trackCountLetterSpacing = -1.0f;
  m_trackCountShadowOffsetX = 2.0f;
  m_trackCountShadowOffsetY = 2.0f;
  m_trackCountShadowOpacity = 0.7f;
  m_playlistHoverWidth = 30;
  m_playlistWidth = 400;
  m_playlistItemOffsetY = 45;
  m_playlistTitleFontSize = 16.0f;
  m_playlistArtistFontSize = 12.0f;
  m_playlistTimeFontSize = 12.0f;

  m_playlistToolbarHeight = 60.0f;
  m_playlistToolbarIconSize = 18.0f;
  m_playlistToolbarIconSpacing = 10.0f;
  m_playlistToolbarTextOffsetY = 30.0f;
  m_playlistToolbarTextFontSize = 12.0f;

  m_playlistPosition = 1;
  m_playlistGripOffset = 10.0f;
  m_playlistGripLineWidth = 2.0f;
  m_playlistGripLineColor = L"#FFFFFF";
  m_playlistGripArrowHeight = 10.0f;
  m_playlistGripArrowWidth = 5.0f;
  m_playlistGripArrowColor = L"#FFFFFF";
  m_playlistGripShadowOffsetX = 2.0f;
  m_playlistGripShadowOffsetY = 2.0f;
  m_playlistGripShadowOpacity = 0.7f;
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
  if (m_iniFilePath.empty()) return false;
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

void ConfigManager::LoadSettings() {
  m_showTitleBar = GetPrivateProfileIntW(L"Window", L"ShowTitleBar", 0,
                                         m_iniFilePath.c_str()) != 0;
  m_showWindowFrame = GetPrivateProfileIntW(L"Window", L"ShowWindowFrame", 0,
                                            m_iniFilePath.c_str()) != 0;
  m_showTaskbar = GetPrivateProfileIntW(L"Window", L"ShowTaskbar", 0,
                                        m_iniFilePath.c_str()) != 0;

  m_zOrder =
      GetPrivateProfileIntW(L"Window", L"ZOrder", 0, m_iniFilePath.c_str());
  m_savePositionOnExit = GetPrivateProfileIntW(L"Window", L"SavePositionOnExit",
                                               1, m_iniFilePath.c_str()) != 0;
  m_enableResize = GetPrivateProfileIntW(L"Window", L"EnableResize", 0,
                                         m_iniFilePath.c_str()) != 0;
  m_lockWindowPosition = GetPrivateProfileIntW(L"Window", L"LockWindowPosition", 0,
                                               m_iniFilePath.c_str()) != 0;

  m_windowX = GetPrivateProfileIntW(L"Window", L"WindowX", CW_USEDEFAULT,
                                    m_iniFilePath.c_str());
  m_windowY = GetPrivateProfileIntW(L"Window", L"WindowY", CW_USEDEFAULT,
                                    m_iniFilePath.c_str());
  m_windowWidth = GetPrivateProfileIntW(L"Window", L"WindowWidth", 1024,
                                        m_iniFilePath.c_str());
  m_windowHeight = GetPrivateProfileIntW(L"Window", L"WindowHeight", 512,
                                         m_iniFilePath.c_str());

  m_enableShadow = GetPrivateProfileIntW(L"Layout_Window", L"EnableShadow", 1,
                                         m_iniFilePath.c_str()) != 0;

  wchar_t buf[32];
  GetPrivateProfileStringW(L"Layout_Window", L"ShadowOffsetX", L"2.0", buf, 32,
                           m_iniFilePath.c_str());
  try {
    m_shadowOffsetX = std::stof(buf);
  } catch (...) {
    m_shadowOffsetX = 2.0f;
  }
  GetPrivateProfileStringW(L"Layout_Window", L"ShadowOffsetY", L"2.0", buf, 32,
                           m_iniFilePath.c_str());
  try {
    m_shadowOffsetY = std::stof(buf);
  } catch (...) {
    m_shadowOffsetY = 2.0f;
  }
  GetPrivateProfileStringW(L"Layout_Window", L"ShadowOpacity", L"0.7", buf, 32,
                           m_iniFilePath.c_str());
  try {
    m_shadowOpacity = std::stof(buf);
  } catch (...) {
    m_shadowOpacity = 0.7f;
  }
  GetPrivateProfileStringW(L"Layout_Window", L"BgDarkenOpacity", L"0.3", buf,
                           32, m_iniFilePath.c_str());
  try {
    m_bgDarkenOpacity = std::stof(buf);
  } catch (...) {
    m_bgDarkenOpacity = 0.3f;
  }
  GetPrivateProfileStringW(L"Layout_Window", L"BgOpacity", L"0.8", buf, 32,
                           m_iniFilePath.c_str());
  try {
    m_bgOpacity = std::stof(buf);
  } catch (...) {
    m_bgOpacity = 0.8f;
  }
  m_backgroundArtMode = GetPrivateProfileIntW(
      L"Layout_Window", L"BackgroundArtMode", 0, m_iniFilePath.c_str());
  GetPrivateProfileStringW(L"Layout_Window", L"ControlHoverHeight", L"50.0",
                           buf, 32, m_iniFilePath.c_str());
  try {
    m_controlHoverHeight = std::stof(buf);
  } catch (...) {
    m_controlHoverHeight = 50.0f;
  }

  GetPrivateProfileStringW(L"Audio", L"DefaultVolume", L"1.0", buf, 32,
                           m_iniFilePath.c_str());
  try {
    m_defaultVolume = std::stof(buf);
  } catch (...) {
    m_defaultVolume = 1.0f;
  }

  m_shuffleMode = GetPrivateProfileIntW(L"Audio", L"ShuffleMode", 1, m_iniFilePath.c_str()) != 0;

  m_volBaseLeftOffset = GetPrivateProfileIntW(
      L"Layout_VolumeControl", L"BaseLeftOffset", 30, m_iniFilePath.c_str());
  m_volBaseBottomOffset = GetPrivateProfileIntW(
      L"Layout_VolumeControl", L"BaseBottomOffset", 22, m_iniFilePath.c_str());
  m_volIconSize = GetPrivateProfileIntW(L"Layout_VolumeControl", L"IconSize",
                                        16, m_iniFilePath.c_str());
  m_volTextOffsetX = GetPrivateProfileIntW(
      L"Layout_VolumeControl", L"TextOffsetX", 31, m_iniFilePath.c_str());
  m_volTextOffsetY = GetPrivateProfileIntW(
      L"Layout_VolumeControl", L"TextOffsetY", -12, m_iniFilePath.c_str());
  GetPrivateProfileStringW(L"Layout_VolumeControl", L"TextLetterSpacing",
                           L"0.0", buf, 32, m_iniFilePath.c_str());
  try {
    m_volTextLetterSpacing = std::stof(buf);
  } catch (...) {
    m_volTextLetterSpacing = 0.0f;
  }
  GetPrivateProfileStringW(L"Layout_VolumeControl", L"FontSize", L"14.0", buf,
                           32, m_iniFilePath.c_str());
  try {
    m_volFontSize = std::stof(buf);
  } catch (...) {
    m_volFontSize = 14.0f;
  }
  GetPrivateProfileStringW(L"Layout_VolumeControl", L"FontFamily", L"Meiryo",
                           buf, 32, m_iniFilePath.c_str());
  m_volFontFamily = buf;

  m_volEnableShadow =
      GetPrivateProfileIntW(L"Layout_VolumeControl", L"EnableShadow", 1,
                            m_iniFilePath.c_str()) != 0;
  GetPrivateProfileStringW(L"Layout_VolumeControl", L"ShadowOffsetX", L"2.0",
                           buf, 32, m_iniFilePath.c_str());
  try {
    m_volShadowOffsetX = std::stof(buf);
  } catch (...) {
    m_volShadowOffsetX = 2.0f;
  }
  GetPrivateProfileStringW(L"Layout_VolumeControl", L"ShadowOffsetY", L"2.0",
                           buf, 32, m_iniFilePath.c_str());
  try {
    m_volShadowOffsetY = std::stof(buf);
  } catch (...) {
    m_volShadowOffsetY = 2.0f;
  }
  GetPrivateProfileStringW(L"Layout_VolumeControl", L"ShadowOpacity", L"0.7",
                           buf, 32, m_iniFilePath.c_str());
  try {
    m_volShadowOpacity = std::stof(buf);
  } catch (...) {
    m_volShadowOpacity = 0.7f;
  }

  m_visualizerMode = GetPrivateProfileIntW(L"Visualizer", L"VisualizerMode", 0,
                                           m_iniFilePath.c_str());

  m_showAppLogo = GetPrivateProfileIntW(L"Visibility", L"ShowAppLogo", 1,
                                        m_iniFilePath.c_str()) != 0;
  m_showNowPlaying = GetPrivateProfileIntW(L"Visibility", L"ShowNowPlaying", 1,
                                           m_iniFilePath.c_str()) != 0;
  m_showNextTrack = GetPrivateProfileIntW(L"Visibility", L"ShowNextTrack", 1,
                                          m_iniFilePath.c_str()) != 0;
  m_showSeekBar = GetPrivateProfileIntW(L"Visibility", L"ShowSeekBar", 1,
                                        m_iniFilePath.c_str()) != 0;
  m_showPlaybackControls =
      GetPrivateProfileIntW(L"Visibility", L"ShowPlaybackControls", 1,
                            m_iniFilePath.c_str()) != 0;
  m_showVolumeControl =
      GetPrivateProfileIntW(L"Visibility", L"ShowVolumeControl", 1,
                            m_iniFilePath.c_str()) != 0;

  m_logoX =
      GetPrivateProfileIntW(L"Layout_AppLogo", L"X", 16, m_iniFilePath.c_str());

  m_logoY =
      GetPrivateProfileIntW(L"Layout_AppLogo", L"Y", 16, m_iniFilePath.c_str());
  m_logoWidth = GetPrivateProfileIntW(L"Layout_AppLogo", L"Width", 64,
                                      m_iniFilePath.c_str());
  m_logoHeight = GetPrivateProfileIntW(L"Layout_AppLogo", L"Height", 64,
                                       m_iniFilePath.c_str());

  GetPrivateProfileStringW(L"Layout_LogoMenu", L"MenuIconSize", L"24.0", buf,
                           32, m_iniFilePath.c_str());
  try {
    m_logoMenuIconSize = std::stof(buf);
  } catch (...) {
    m_logoMenuIconSize = 24.0f;
  }

  m_logoMenuIconSpacing = GetPrivateProfileIntW(
      L"Layout_LogoMenu", L"MenuIconSpacing", 40, m_iniFilePath.c_str());

  m_logoMenuIconOffsetX = GetPrivateProfileIntW(
      L"Layout_LogoMenu", L"MenuIconOffsetX", 0, m_iniFilePath.c_str());

  m_logoMenuIconOffsetY = GetPrivateProfileIntW(
      L"Layout_LogoMenu", L"MenuIconOffsetY", 0, m_iniFilePath.c_str());

  GetPrivateProfileStringW(L"Layout_LogoMenu", L"MenuScrollDuration", L"0.5",
                           buf, 32, m_iniFilePath.c_str());
  try {
    m_logoMenuScrollDuration = std::stof(buf);
  } catch (...) {
    m_logoMenuScrollDuration = 0.5f;
  }

  wchar_t logoFontBuf[256];
  GetPrivateProfileStringW(L"Layout_LogoMenu", L"MenuFontFamily",
                           L"Segoe UI Emoji", logoFontBuf, 256,
                           m_iniFilePath.c_str());
  m_logoMenuFontFamily = logoFontBuf;

  GetPrivateProfileStringW(L"Layout_LogoMenu", L"MenuTextColor", L"#FFFFFF",
                           logoFontBuf, 256, m_iniFilePath.c_str());
  m_logoMenuTextColor = logoFontBuf;

  GetPrivateProfileStringW(L"Layout_LogoMenu", L"MenuTypingFontFamily",
                           L"Consolas", logoFontBuf, 256,
                           m_iniFilePath.c_str());
  m_logoMenuTypingFontFamily = logoFontBuf;

  GetPrivateProfileStringW(L"Layout_LogoMenu", L"MenuTypingFontSize", L"14.0",
                           buf, 32, m_iniFilePath.c_str());
  try {
    m_logoMenuTypingFontSize = std::stof(buf);
  } catch (...) {
    m_logoMenuTypingFontSize = 14.0f;
  }

  m_logoMenuTextOffsetX = GetPrivateProfileIntW(
      L"Layout_LogoMenu", L"MenuTextOffsetX", 0, m_iniFilePath.c_str());

  m_logoMenuTextOffsetY = GetPrivateProfileIntW(
      L"Layout_LogoMenu", L"MenuTextOffsetY", 60, m_iniFilePath.c_str());

  GetPrivateProfileStringW(L"Layout_LogoMenu", L"MenuTypingLetterSpacing",
                           L"0.0", buf, 32, m_iniFilePath.c_str());
  try {
    m_logoMenuTypingLetterSpacing = std::stof(buf);
  } catch (...) {
    m_logoMenuTypingLetterSpacing = 0.0f;
  }

  GetPrivateProfileStringW(L"Layout_LogoMenu", L"MenuStrikeLength", L"20.0",
                           buf, 32, m_iniFilePath.c_str());
  try {
    m_logoMenuStrikeLength = std::stof(buf);
  } catch (...) {
    m_logoMenuStrikeLength = 20.0f;
  }

  GetPrivateProfileStringW(L"Layout_LogoMenu", L"MenuStrikeThickness", L"2.0",
                           buf, 32, m_iniFilePath.c_str());
  try {
    m_logoMenuStrikeThickness = std::stof(buf);
  } catch (...) {
    m_logoMenuStrikeThickness = 2.0f;
  }

  GetPrivateProfileStringW(L"Layout_LogoMenu", L"IconHoverBgAlpha", L"0.6", buf,
                           32, m_iniFilePath.c_str());
  try {
    m_logoMenuIconHoverBgAlpha = std::stof(buf);
  } catch (...) {
    m_logoMenuIconHoverBgAlpha = 0.6f;
  }

  GetPrivateProfileStringW(L"Layout_LogoMenu", L"VisualizerIconFontSize",
                           L"12.0", buf, 32, m_iniFilePath.c_str());
  try {
    m_logoMenuVisualizerFontSize = std::stof(buf);
  } catch (...) {
    m_logoMenuVisualizerFontSize = 12.0f;
  }

  m_logoMenuVisualizerIconOffsetX = GetPrivateProfileIntW(
      L"Layout_LogoMenu", L"VisualizerIconOffsetX", 12, m_iniFilePath.c_str());

  m_logoMenuVisualizerIconOffsetY = GetPrivateProfileIntW(
      L"Layout_LogoMenu", L"VisualizerIconOffsetY", 7, m_iniFilePath.c_str());

  GetPrivateProfileStringW(L"Layout_LogoMenu", L"DescShadowOffsetX", L"2.0",
                           buf, 32, m_iniFilePath.c_str());
  try {
    m_logoMenuDescShadowOffsetX = std::stof(buf);
  } catch (...) {
    m_logoMenuDescShadowOffsetX = 2.0f;
  }

  GetPrivateProfileStringW(L"Layout_LogoMenu", L"DescShadowOffsetY", L"2.0",
                           buf, 32, m_iniFilePath.c_str());
  try {
    m_logoMenuDescShadowOffsetY = std::stof(buf);
  } catch (...) {
    m_logoMenuDescShadowOffsetY = 2.0f;
  }

  GetPrivateProfileStringW(L"Layout_LogoMenu", L"DescShadowOpacity", L"0.7",
                           buf, 32, m_iniFilePath.c_str());
  try {
    m_logoMenuDescShadowOpacity = std::stof(buf);
  } catch (...) {
    m_logoMenuDescShadowOpacity = 0.7f;
  }

  m_baseX = GetPrivateProfileIntW(L"Layout_NowPlaying", L"BaseX", 30,
                                  m_iniFilePath.c_str());

  m_baseBottomOffset = GetPrivateProfileIntW(
      L"Layout_NowPlaying", L"BaseBottomOffset", 162, m_iniFilePath.c_str());
  m_artOffsetX = GetPrivateProfileIntW(L"Layout_NowPlaying", L"ArtOffsetX", 0,
                                       m_iniFilePath.c_str());
  m_artOffsetY = GetPrivateProfileIntW(L"Layout_NowPlaying", L"ArtOffsetY", 0,
                                       m_iniFilePath.c_str());
  m_artSize = GetPrivateProfileIntW(L"Layout_NowPlaying", L"ArtSize", 120,
                                    m_iniFilePath.c_str());

  GetPrivateProfileStringW(L"Layout_NowPlaying", L"FallbackArtOpacity", L"0.5",
                           buf, 32, m_iniFilePath.c_str());
  try {
    m_fallbackArtOpacity = std::stof(buf);
  } catch (...) {
    m_fallbackArtOpacity = 0.5f;
  }

  m_titleOffsetX = GetPrivateProfileIntW(L"Layout_NowPlaying", L"TitleOffsetX",
                                         140, m_iniFilePath.c_str());
  m_titleOffsetY = GetPrivateProfileIntW(L"Layout_NowPlaying", L"TitleOffsetY",
                                         10, m_iniFilePath.c_str());
  GetPrivateProfileStringW(L"Layout_NowPlaying", L"TitleFontSize", L"32.0", buf,
                           32, m_iniFilePath.c_str());
  try {
    m_titleFontSize = std::stof(buf);
  } catch (...) {
    m_titleFontSize = 32.0f;
  }
  GetPrivateProfileStringW(L"Layout_NowPlaying", L"TitleFontFamily", L"Meiryo",
                           buf, 32, m_iniFilePath.c_str());
  m_titleFontFamily = buf;

  m_artistOffsetX = GetPrivateProfileIntW(
      L"Layout_NowPlaying", L"ArtistOffsetX", 140, m_iniFilePath.c_str());
  m_artistOffsetY = GetPrivateProfileIntW(
      L"Layout_NowPlaying", L"ArtistOffsetY", 55, m_iniFilePath.c_str());
  GetPrivateProfileStringW(L"Layout_NowPlaying", L"ArtistFontSize", L"18.0",
                           buf, 32, m_iniFilePath.c_str());
  try {
    m_artistFontSize = std::stof(buf);
  } catch (...) {
    m_artistFontSize = 18.0f;
  }
  GetPrivateProfileStringW(L"Layout_NowPlaying", L"ArtistFontFamily", L"Meiryo",
                           buf, 32, m_iniFilePath.c_str());
  m_artistFontFamily = buf;

  GetPrivateProfileStringW(L"Layout_SeekBar", L"SeekBarMargin", L"20.0", buf,
                           32, m_iniFilePath.c_str());

  try {
    m_seekBarMargin = std::stof(buf);
  } catch (...) {
    m_seekBarMargin = 20.0f;
  }

  m_seekBarHeight = GetPrivateProfileIntW(L"Layout_SeekBar", L"Height", 3,
                                          m_iniFilePath.c_str());
  m_seekBarBottomOffset = GetPrivateProfileIntW(
      L"Layout_SeekBar", L"BottomOffset", 50, m_iniFilePath.c_str());

  GetPrivateProfileStringW(L"Layout_SeekBar", L"BgOpacity", L"0.3", buf, 32,
                           m_iniFilePath.c_str());
  try {
    m_seekBarBgOpacity = std::stof(buf);
  } catch (...) {
    m_seekBarBgOpacity = 0.3f;
  }

  GetPrivateProfileStringW(L"Layout_SeekBar", L"TimeFontFamily", L"Consolas",
                           buf, 32, m_iniFilePath.c_str());
  m_seekBarTimeFontFamily = buf;

  GetPrivateProfileStringW(L"Layout_SeekBar", L"TimeFontSize", L"12.0", buf, 32,
                           m_iniFilePath.c_str());
  try {
    m_seekBarTimeFontSize = std::stof(buf);
  } catch (...) {
    m_seekBarTimeFontSize = 12.0f;
  }

  m_seekBarTimeAreaWidth = GetPrivateProfileIntW(
      L"Layout_SeekBar", L"TimeAreaWidth", 100, m_iniFilePath.c_str());

  GetPrivateProfileStringW(L"Layout_SeekBar", L"TimeLetterSpacing", L"0.0", buf,
                           32, m_iniFilePath.c_str());
  try {
    m_seekBarTimeLetterSpacing = std::stof(buf);
  } catch (...) {
    m_seekBarTimeLetterSpacing = 0.0f;
  }

  m_enableNextTrack =
      GetPrivateProfileIntW(L"Layout_NextTrack", L"EnableNextTrack", 0,
                            m_iniFilePath.c_str()) != 0;
  m_nextBaseRightOffset = GetPrivateProfileIntW(
      L"Layout_NextTrack", L"BaseRightOffset", 250, m_iniFilePath.c_str());
  m_nextBaseBottomOffset = GetPrivateProfileIntW(
      L"Layout_NextTrack", L"BaseBottomOffset", 80, m_iniFilePath.c_str());
  m_nextArtOffsetX = GetPrivateProfileIntW(L"Layout_NextTrack", L"ArtOffsetX",
                                           0, m_iniFilePath.c_str());
  m_nextArtOffsetY = GetPrivateProfileIntW(L"Layout_NextTrack", L"ArtOffsetY",
                                           0, m_iniFilePath.c_str());
  m_nextArtSize = GetPrivateProfileIntW(L"Layout_NextTrack", L"ArtSize", 40,
                                        m_iniFilePath.c_str());

  GetPrivateProfileStringW(L"Layout_NextTrack", L"BgOpacity", L"0.3", buf, 32,
                           m_iniFilePath.c_str());
  try {
    m_nextBgOpacity = std::stof(buf);
  } catch (...) {
    m_nextBgOpacity = 0.3f;
  }

  GetPrivateProfileStringW(L"Layout_NextTrack", L"FallbackArtOpacity", L"0.5",
                           buf, 32, m_iniFilePath.c_str());
  try {
    m_nextFallbackArtOpacity = std::stof(buf);
  } catch (...) {
    m_nextFallbackArtOpacity = 0.5f;
  }

  m_nextLabelOffsetX = GetPrivateProfileIntW(
      L"Layout_NextTrack", L"LabelOffsetX", 0, m_iniFilePath.c_str());
  m_nextLabelOffsetY = GetPrivateProfileIntW(
      L"Layout_NextTrack", L"LabelOffsetY", -20, m_iniFilePath.c_str());
  GetPrivateProfileStringW(L"Layout_NextTrack", L"LabelFontSize", L"12.0", buf,
                           32, m_iniFilePath.c_str());
  try {
    m_nextLabelFontSize = std::stof(buf);
  } catch (...) {
    m_nextLabelFontSize = 12.0f;
  }
  GetPrivateProfileStringW(L"Layout_NextTrack", L"LabelFontFamily", L"Meiryo",
                           buf, 32, m_iniFilePath.c_str());
  m_nextLabelFontFamily = buf;

  m_nextTitleOffsetX = GetPrivateProfileIntW(
      L"Layout_NextTrack", L"TitleOffsetX", 50, m_iniFilePath.c_str());
  m_nextTitleOffsetY = GetPrivateProfileIntW(
      L"Layout_NextTrack", L"TitleOffsetY", 0, m_iniFilePath.c_str());
  GetPrivateProfileStringW(L"Layout_NextTrack", L"TitleFontSize", L"14.0", buf,
                           32, m_iniFilePath.c_str());
  try {
    m_nextTitleFontSize = std::stof(buf);
  } catch (...) {
    m_nextTitleFontSize = 14.0f;
  }
  GetPrivateProfileStringW(L"Layout_NextTrack", L"TitleFontFamily", L"Meiryo",
                           buf, 32, m_iniFilePath.c_str());
  m_nextTitleFontFamily = buf;

  m_nextArtistOffsetX = GetPrivateProfileIntW(
      L"Layout_NextTrack", L"ArtistOffsetX", 50, m_iniFilePath.c_str());
  m_nextArtistOffsetY = GetPrivateProfileIntW(
      L"Layout_NextTrack", L"ArtistOffsetY", 20, m_iniFilePath.c_str());
  GetPrivateProfileStringW(L"Layout_NextTrack", L"ArtistFontSize", L"12.0", buf,
                           32, m_iniFilePath.c_str());
  try {
    m_nextArtistFontSize = std::stof(buf);
  } catch (...) {
    m_nextArtistFontSize = 12.0f;
  }
  GetPrivateProfileStringW(L"Layout_NextTrack", L"ArtistFontFamily", L"Meiryo",
                           buf, 32, m_iniFilePath.c_str());
  m_nextArtistFontFamily = buf;

  m_playbackBaseBottomOffset =
      GetPrivateProfileIntW(L"Layout_PlaybackControls", L"BaseBottomOffset", 40,
                            m_iniFilePath.c_str());
  m_playbackCenterOffsetX = GetPrivateProfileIntW(
      L"Layout_PlaybackControls", L"CenterOffsetX", 0, m_iniFilePath.c_str());
  m_playbackButtonSpacing = GetPrivateProfileIntW(
      L"Layout_PlaybackControls", L"ButtonSpacing", 40, m_iniFilePath.c_str());
  m_playbackButtonSize = GetPrivateProfileIntW(
      L"Layout_PlaybackControls", L"ButtonSize", 20, m_iniFilePath.c_str());

  GetPrivateProfileStringW(L"Layout_NowPlaying", L"TrackCountFontFamily",
                           L"Courier New", buf, 32, m_iniFilePath.c_str());
  m_trackCountFontFamily = buf;
  GetPrivateProfileStringW(L"Layout_NowPlaying", L"TrackCountFontSize", L"14.0",
                           buf, 32, m_iniFilePath.c_str());
  try {
    m_trackCountFontSize = std::stof(buf);
  } catch (...) {
    m_trackCountFontSize = 14.0f;
  }
  m_trackCountOffsetX = GetPrivateProfileIntW(
      L"Layout_NowPlaying", L"TrackCountOffsetX", 20, m_iniFilePath.c_str());
  m_trackCountOffsetY = GetPrivateProfileIntW(
      L"Layout_NowPlaying", L"TrackCountOffsetY", 100, m_iniFilePath.c_str());
  m_trackCountTextAlignment =
      GetPrivateProfileIntW(L"Layout_NowPlaying", L"TrackCountTextAlignment", 1,
                            m_iniFilePath.c_str());
  GetPrivateProfileStringW(L"Layout_NowPlaying", L"TrackCountLetterSpacing",
                           L"-1.0", buf, 32, m_iniFilePath.c_str());
  try {
    m_trackCountLetterSpacing = std::stof(buf);
  } catch (...) {
    m_trackCountLetterSpacing = -1.0f;
  }
  GetPrivateProfileStringW(L"Layout_NowPlaying", L"TrackCountShadowOffsetX",
                           L"2.0", buf, 32, m_iniFilePath.c_str());
  try {
    m_trackCountShadowOffsetX = std::stof(buf);
  } catch (...) {
    m_trackCountShadowOffsetX = 2.0f;
  }
  GetPrivateProfileStringW(L"Layout_NowPlaying", L"TrackCountShadowOffsetY",
                           L"2.0", buf, 32, m_iniFilePath.c_str());
  try {
    m_trackCountShadowOffsetY = std::stof(buf);
  } catch (...) {
    m_trackCountShadowOffsetY = 2.0f;
  }
  GetPrivateProfileStringW(L"Layout_NowPlaying", L"TrackCountShadowOpacity",
                           L"0.7", buf, 32, m_iniFilePath.c_str());
  try {
    m_trackCountShadowOpacity = std::stof(buf);
  } catch (...) {
    m_trackCountShadowOpacity = 0.7f;
  }

  m_playlistHoverWidth = GetPrivateProfileIntW(
      L"Layout_Playlist", L"PlaylistHoverWidth", 30, m_iniFilePath.c_str());

  m_playlistPosition = GetPrivateProfileIntW(
      L"Layout_Playlist", L"PlaylistPosition", 1, m_iniFilePath.c_str());

  m_playlistWidth = GetPrivateProfileIntW(L"Layout_Playlist", L"PlaylistWidth",
                                          400, m_iniFilePath.c_str());
  m_playlistItemOffsetY = GetPrivateProfileIntW(
      L"Layout_Playlist", L"PlaylistItemOffsetY", 45, m_iniFilePath.c_str());
  GetPrivateProfileStringW(L"Layout_Playlist", L"PlaylistTitleFontSize",
                           L"16.0", buf, 32, m_iniFilePath.c_str());
  try {
    m_playlistTitleFontSize = std::stof(buf);
  } catch (...) {
    m_playlistTitleFontSize = 16.0f;
  }

  GetPrivateProfileStringW(L"Layout_Playlist", L"PlaylistArtistFontSize",
                           L"12.0", buf, 32, m_iniFilePath.c_str());
  try {
    m_playlistArtistFontSize = std::stof(buf);
  } catch (...) {
    m_playlistArtistFontSize = 12.0f;
  }

  GetPrivateProfileStringW(L"Layout_Playlist", L"PlaylistTimeFontSize", L"12.0",
                           buf, 32, m_iniFilePath.c_str());
  try {
    m_playlistTimeFontSize = std::stof(buf);
  } catch (...) {
    m_playlistTimeFontSize = 12.0f;
  }

  wchar_t fontBuf[256];
  GetPrivateProfileStringW(L"Layout_Playlist", L"PlaylistTitleFontFamily",
                           L"Meiryo", fontBuf, 256, m_iniFilePath.c_str());
  m_playlistTitleFontFamily = fontBuf;

  GetPrivateProfileStringW(L"Layout_Playlist", L"PlaylistArtistFontFamily",
                           L"Meiryo", fontBuf, 256, m_iniFilePath.c_str());
  m_playlistArtistFontFamily = fontBuf;

  GetPrivateProfileStringW(L"Layout_Playlist", L"PlaylistTimeFontFamily",
                           L"Courier New", fontBuf, 256, m_iniFilePath.c_str());
  m_playlistTimeFontFamily = fontBuf;

  GetPrivateProfileStringW(L"Layout_Playlist", L"PlaylistTimeLetterSpacing",
                           L"0.0", buf, 32, m_iniFilePath.c_str());
  try {
    m_playlistTimeLetterSpacing = std::stof(buf);
  } catch (...) {
    m_playlistTimeLetterSpacing = 0.0f;
  }

  GetPrivateProfileStringW(L"Layout_Playlist", L"PlaylistArtistColor",
                           L"#AAAAAA", fontBuf, 256, m_iniFilePath.c_str());
  m_playlistArtistColor = fontBuf;

  GetPrivateProfileStringW(L"Layout_Playlist", L"PlaylistTimeColor", L"#FFFFFF",
                           fontBuf, 256, m_iniFilePath.c_str());
  m_playlistTimeColor = fontBuf;

  GetPrivateProfileStringW(L"Layout_Playlist", L"PlaylistBgOpacity", L"0.8",
                           buf, 32, m_iniFilePath.c_str());
  try {
    m_playlistBgOpacity = std::stof(buf);
  } catch (...) {
    m_playlistBgOpacity = 0.8f;
  }

  GetPrivateProfileStringW(L"Layout_Playlist", L"PlaylistGripOffset", L"10.0",
                           buf, 32, m_iniFilePath.c_str());
  try {
    m_playlistGripOffset = std::stof(buf);
  } catch (...) {
    m_playlistGripOffset = 10.0f;
  }

  GetPrivateProfileStringW(L"Layout_Playlist", L"PlaylistGripLineWidth", L"2.0",
                           buf, 32, m_iniFilePath.c_str());
  try {
    m_playlistGripLineWidth = std::stof(buf);
  } catch (...) {
    m_playlistGripLineWidth = 2.0f;
  }

  GetPrivateProfileStringW(L"Layout_Playlist", L"PlaylistGripLineColor",
                           L"#FFFFFF", fontBuf, 256, m_iniFilePath.c_str());
  m_playlistGripLineColor = fontBuf;

  GetPrivateProfileStringW(L"Layout_Playlist", L"PlaylistGripArrowHeight",
                           L"10.0", buf, 32, m_iniFilePath.c_str());
  try {
    m_playlistGripArrowHeight = std::stof(buf);
  } catch (...) {
    m_playlistGripArrowHeight = 10.0f;
  }

  GetPrivateProfileStringW(L"Layout_Playlist", L"PlaylistGripArrowWidth",
                           L"5.0", buf, 32, m_iniFilePath.c_str());
  try {
    m_playlistGripArrowWidth = std::stof(buf);
  } catch (...) {
    m_playlistGripArrowWidth = 5.0f;
  }

  GetPrivateProfileStringW(L"Layout_Playlist", L"PlaylistGripArrowColor",
                           L"#FFFFFF", fontBuf, 256, m_iniFilePath.c_str());
  m_playlistGripArrowColor = fontBuf;

  GetPrivateProfileStringW(L"Layout_Playlist", L"PlaylistGripShadowOffsetX",
                           L"2.0", buf, 32, m_iniFilePath.c_str());
  try {
    m_playlistGripShadowOffsetX = std::stof(buf);
  } catch (...) {
    m_playlistGripShadowOffsetX = 2.0f;
  }

  GetPrivateProfileStringW(L"Layout_Playlist", L"PlaylistGripShadowOffsetY",
                           L"2.0", buf, 32, m_iniFilePath.c_str());
  try {
    m_playlistGripShadowOffsetY = std::stof(buf);
  } catch (...) {
    m_playlistGripShadowOffsetY = 2.0f;
  }

  GetPrivateProfileStringW(L"Layout_Playlist", L"PlaylistGripShadowOpacity",
                           L"0.7", buf, 32, m_iniFilePath.c_str());
  try {
    m_playlistGripShadowOpacity = std::stof(buf);
  } catch (...) {
    m_playlistGripShadowOpacity = 0.7f;
  }

  m_playlistTitleOffsetX = GetPrivateProfileIntW(
      L"Layout_Playlist", L"PlaylistTitleOffsetX", 20, m_iniFilePath.c_str());
  m_playlistTitleOffsetY = GetPrivateProfileIntW(
      L"Layout_Playlist", L"PlaylistTitleOffsetY", 10, m_iniFilePath.c_str());

  m_playlistArtistOffsetX = GetPrivateProfileIntW(
      L"Layout_Playlist", L"PlaylistArtistOffsetX", 20, m_iniFilePath.c_str());
  m_playlistArtistOffsetY = GetPrivateProfileIntW(
      L"Layout_Playlist", L"PlaylistArtistOffsetY", 35, m_iniFilePath.c_str());

  m_playlistTimeOffsetX = GetPrivateProfileIntW(
      L"Layout_Playlist", L"PlaylistTimeOffsetX", 20, m_iniFilePath.c_str());
  m_playlistTimeOffsetY = GetPrivateProfileIntW(
      L"Layout_Playlist", L"PlaylistTimeOffsetY", 35, m_iniFilePath.c_str());

  GetPrivateProfileStringW(L"Layout_Playlist", L"ToolbarHeight", L"60.0", buf, 32, m_iniFilePath.c_str());
  try { m_playlistToolbarHeight = std::stof(buf); } catch (...) { m_playlistToolbarHeight = 60.0f; }

  GetPrivateProfileStringW(L"Layout_Playlist", L"ToolbarIconSize", L"18.0", buf, 32, m_iniFilePath.c_str());
  try { m_playlistToolbarIconSize = std::stof(buf); } catch (...) { m_playlistToolbarIconSize = 18.0f; }

  GetPrivateProfileStringW(L"Layout_Playlist", L"ToolbarIconSpacing", L"10.0", buf, 32, m_iniFilePath.c_str());
  try { m_playlistToolbarIconSpacing = std::stof(buf); } catch (...) { m_playlistToolbarIconSpacing = 10.0f; }

  GetPrivateProfileStringW(L"Layout_Playlist", L"ToolbarTextOffsetY", L"30.0", buf, 32, m_iniFilePath.c_str());
  try { m_playlistToolbarTextOffsetY = std::stof(buf); } catch (...) { m_playlistToolbarTextOffsetY = 30.0f; }

  GetPrivateProfileStringW(L"Layout_Playlist", L"ToolbarTextFontSize", L"12.0", buf, 32, m_iniFilePath.c_str());
  try { m_playlistToolbarTextFontSize = std::stof(buf); } catch (...) { m_playlistToolbarTextFontSize = 12.0f; }

  wchar_t pathBuf[MAX_PATH];

  GetPrivateProfileStringW(L"Playlist", L"DefaultPlaylistPath", L"", pathBuf,
                           MAX_PATH, m_iniFilePath.c_str());
  std::wstring loadedPlaylistPath = pathBuf;
  if (loadedPlaylistPath.empty()) {
    std::wstring exePath = GetExecutablePath();
    size_t pos = exePath.find_last_of(L"\\/");
    if (pos != std::wstring::npos) {
      m_defaultPlaylistPath = exePath.substr(0, pos) + L"\\oztone_playlist.lst";
    } else {
      m_defaultPlaylistPath = L"oztone_playlist.lst";
    }
    WritePrivateProfileStringW(L"Playlist", L"DefaultPlaylistPath",
                               m_defaultPlaylistPath.c_str(),
                               m_iniFilePath.c_str());
  } else {
    m_defaultPlaylistPath = loadedPlaylistPath;
  }
}

void ConfigManager::SaveDefaultSettings() {
  std::ofstream ofs(m_iniFilePath);
  if (ofs) {
    ofs << DEFAULT_INI_CONTENT;
    ofs.close();
  }

  std::wstring exePath = GetExecutablePath();
  std::wstring defPlaylistPath;
  size_t pos = exePath.find_last_of(L"\\/");
  if (pos != std::wstring::npos) {
    defPlaylistPath = exePath.substr(0, pos) + L"\\oztone_playlist.lst";
  } else {
    defPlaylistPath = L"oztone_playlist.lst";
  }
  WritePrivateProfileStringW(L"Playlist", L"DefaultPlaylistPath",
                             defPlaylistPath.c_str(), m_iniFilePath.c_str());
}

void ConfigManager::SaveWindowPosition(int x, int y, int width, int height) {
  m_windowX = x;
  m_windowY = y;
  m_windowWidth = width;
  m_windowHeight = height;

  WritePrivateProfileStringW(L"Window", L"WindowX", std::to_wstring(x).c_str(),
                             m_iniFilePath.c_str());
  WritePrivateProfileStringW(L"Window", L"WindowY", std::to_wstring(y).c_str(),
                             m_iniFilePath.c_str());
  WritePrivateProfileStringW(L"Window", L"WindowWidth",
                             std::to_wstring(width).c_str(),
                             m_iniFilePath.c_str());
  WritePrivateProfileStringW(L"Window", L"WindowHeight",
                             std::to_wstring(height).c_str(),
                             m_iniFilePath.c_str());
}

void ConfigManager::SetZOrder(int zOrder) {
  m_zOrder = zOrder;
  WritePrivateProfileStringW(L"Window", L"ZOrder",
                             std::to_wstring(zOrder).c_str(),
                             m_iniFilePath.c_str());
}

void ConfigManager::SetSavePositionOnExit(bool save) {
  m_savePositionOnExit = save;
  WritePrivateProfileStringW(L"Window", L"SavePositionOnExit",
                             save ? L"1" : L"0", m_iniFilePath.c_str());
}

void ConfigManager::SetEnableResize(bool enable) {
  m_enableResize = enable;
  WritePrivateProfileStringW(L"Window", L"EnableResize", enable ? L"1" : L"0",
                             m_iniFilePath.c_str());
}

void ConfigManager::SetLockWindowPosition(bool lock) {
  m_lockWindowPosition = lock;
  WritePrivateProfileStringW(L"Window", L"LockWindowPosition", lock ? L"1" : L"0",
                             m_iniFilePath.c_str());
}

void ConfigManager::SetBackgroundArtMode(int mode) {
  m_backgroundArtMode = mode;
  WritePrivateProfileStringW(L"Layout_Window", L"BackgroundArtMode",
                             std::to_wstring(mode).c_str(),
                             m_iniFilePath.c_str());
}

void ConfigManager::SetVisualizerMode(int mode) {
  m_visualizerMode = mode;
  WritePrivateProfileStringW(L"Visualizer", L"VisualizerMode",
                             std::to_wstring(mode).c_str(),
                             m_iniFilePath.c_str());
}

void ConfigManager::SetDefaultVolume(float volume) {
  m_defaultVolume = volume;
  WritePrivateProfileStringW(L"Audio", L"DefaultVolume",
                             std::to_wstring(volume).c_str(),
                             m_iniFilePath.c_str());
}

void ConfigManager::SetShuffleMode(bool mode) {
  m_shuffleMode = mode;
  WritePrivateProfileStringW(L"Audio", L"ShuffleMode", mode ? L"1" : L"0", m_iniFilePath.c_str());
}

void ConfigManager::SetDefaultPlaylistPath(const std::wstring &path) {
  m_defaultPlaylistPath = path;
  WritePrivateProfileStringW(L"Playlist", L"DefaultPlaylistPath", path.c_str(),
                             m_iniFilePath.c_str());
}

void ConfigManager::SetPlaylistPosition(int position) {
  m_playlistPosition = position;
  WritePrivateProfileStringW(L"Layout_Playlist", L"PlaylistPosition",
                             std::to_wstring(position).c_str(),
                             m_iniFilePath.c_str());
}

std::vector<std::wstring> ConfigManager::GetAvailablePlaylists() const {
  std::vector<std::wstring> playlists;
  try {
    std::filesystem::path currentPath(m_defaultPlaylistPath);
    std::filesystem::path dir = currentPath.parent_path();

    if (dir.empty()) {
      std::wstring exePath = GetExecutablePath();
      if (!exePath.empty()) {
        size_t pos = exePath.find_last_of(L"\\/");
        if (pos != std::wstring::npos) {
          dir = exePath.substr(0, pos);
        }
      }
      if (dir.empty()) {
        dir = std::filesystem::current_path();
      }
    }

    if (std::filesystem::exists(dir) && std::filesystem::is_directory(dir)) {
      for (const auto &entry : std::filesystem::directory_iterator(dir)) {
        if (entry.is_regular_file()) {
          std::wstring ext = entry.path().extension().wstring();
          std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
          if (ext == L".ozl" || ext == L".lst") {
            playlists.push_back(entry.path().wstring());
          }
        }
      }
    }
  } catch (...) {
    // ignore errors
  }

  std::sort(playlists.begin(), playlists.end());
  return playlists;
}

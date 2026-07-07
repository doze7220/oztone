#include "ConfigManager.h"
#include <algorithm>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
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
HoverIconColor=#2452a6
HoverFadeOutSpeed=2
ControlLeaveDelay=3.0

[Layout_AppLogo]
X=16
Y=16
Width=64
Height=64

[Layout_LogoMenu]
MenuTypingFontSize=14.0
MenuTypingFontFamily=Meiryo
MenuTextColor=#FFFFFF
MenuTextOffsetX=8
MenuTextOffsetY=-18
MenuTypingLetterSpacing=-1.0
MenuIconSize=44.0
MenuIconSpacing=54
MenuIconOffsetX=-24
MenuIconOffsetY=38
MenuScrollDuration=0.5
MenuFontFamily="Segoe UI Emoji"
IconHoverBgAlpha=0.4
VisualizerIconFontSize=24.0
VisualizerIconOffsetX=6
VisualizerIconOffsetY=0
LockIconFontSize=20.0
LockIconOffsetX=-2
LockIconOffsetY=-2
DescShadowOffsetX=2.0
DescShadowOffsetY=1.0
DescShadowOpacity=0.8
MenuLeaveDelay=2

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
SkipIconPoints=0.1,-0.5,0.65,-0.5,-0.15,0.0,0.65,0.5,0.1,0.5,-0.65,0.0
SkipTextFontSize=15.0
SkipTextOffsetX=0.04
SkipTextOffsetY=-0.05
SkipTextShadowColor=#444444
SkipTextShadowOpacity=0.40
SkipTextShadowShift=1.00

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
TooltipText="🖱️↕"
TooltipFontSize=20.0
TooltipFontFamily="Segoe UI Emoji"
TooltipOffsetY=-5.0
TooltipWidth=65.0
TooltipHeight=40.0
TooltipBgColor=#A0000000
TooltipBgOpacity=1.0
TooltipTextColor=#FFFFFF

[Audio]
DefaultVolume=1.0
ShuffleMode=1

[Visualizer]
VisualizerMode=2

[Layout_Playlist]
PlaylistPosition=1
IsPlaylistPinned=0
PlaylistHoverWidth=120
PlaylistWidth=250
PlaylistItemOffsetY=45
PlayingItemColor=#FFA500
HoverItemColor=#a3a362
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
PinSubIconOffsetX=6
PinSubIconOffsetY=6
PinSubIconFontSize=10.0
PlaylistLeaveDelay=0.5

[Layout_GlobalHotkeys]
FontFamily=MS Gothic
FontSize=15.0
LineSpacing=20.0
CoreColor=#FFFFFF
GlowColor=#00FFFF
GlowOpacity=0.10
KeyColumnOffset=360.0
ActionColumnOffset=140.0
ShadowColor=#000000
ShadowOpacity=0.2

[Layout_OSD]
OsdFontFamily=Meiryo
OsdFontSize=48.0
OsdTextColor=#FFFFFF
OsdShadowColor=#000000
OsdShadowOffsetX=2.0
OsdShadowOffsetY=2.0
OsdShadowOpacity=0.8
OsdFadeWait=1.5
OsdFadeSpeed=1.5

[GlobalHotkeys]
ShowHotkeys=0

; =========================================================
; Modifiers (修飾キー): 1=ALT, 2=CTRL, 4=SHIFT, 8=WIN 
; （論理和で組み合わせ可能。例: CTRL(2) + SHIFT(4) = 6, CTRL(2) + SHIFT(4) + ALT(1) = 7）
; VK: 仮想キーコード (33=PgUp, 34=PgDn, 35=End, 36=Home, 37=Left, 38=Up, 39=Right, 40=Down, 46=Del)
; =========================================================
; --- 再生コントロール (CTRL + ALT + WIN + 矢印) ---
;プレイコントロール：次の曲
Modifier_NextTrack=11
VK_NextTrack=39

;プレイコントロール：前の曲
Modifier_PrevTrack=11
VK_PrevTrack=37

;プレイコントロール：再生/一時停止
Modifier_PlayPause=11
VK_PlayPause=38

;プレイコントロール：停止
Modifier_Stop=11
VK_Stop=40

; --- 音量コントロール (CTRL + ALT + WIN + Home/End,PageUP/PageDown) ---
;音量操作：5%アップ
Modifier_VolUp5=11
VK_VolUp5=36

;音量操作：5%ダウン
Modifier_VolDown5=11
VK_VolDown5=35

;音量操作：25%アップ
Modifier_VolUp25=11
VK_VolUp25=33

;音量操作：25%ダウン
Modifier_VolDown25=11
VK_VolDown25=34

; --- プレイリスト切替 (CTRL + WIN + PageUp/PageDown) ---
;前のプレイリスト（最初のプレイリストの場合は最後のプレイリストへ）
Modifier_PrevPlaylist=10
VK_PrevPlaylist=33

;次のプレイリスト（最後のプレイリストの場合は先頭のプレイリストへ）
Modifier_NextPlaylist=10
VK_NextPlaylist=34

; --- ウィンドウアクティブ＆Z-Order制御 (CTRL + WIN + Home/End) ---
;ウィンドウ最前面固定化＆アクティブ化
Modifier_ActiveTopMost=10
VK_ActiveTopMost=36

;ウィンドウ最背面固定化＆アクティブ化
Modifier_ActiveBottom=10
VK_ActiveBottom=35

; --- アプリ終了 (CTRL + WIN + Delete) ---
Modifier_ExitApp=10
VK_ExitApp=46

)";

ConfigManager::ConfigManager()
    : m_showTitleBar(false), m_showWindowFrame(false), m_showTaskbar(false),
      m_showAppLogo(true), m_showNowPlaying(true), m_showNextTrack(true),
      m_enableNextTrack(false), m_showSeekBar(true),
      m_showPlaybackControls(true), m_showVolumeControl(true), m_zOrder(0),
      m_savePositionOnExit(true), m_enableResize(false),
      m_lockWindowPosition(false), m_shuffleMode(true),
      m_isPlaylistPinned(false), m_windowX(CW_USEDEFAULT),
      m_windowY(CW_USEDEFAULT), m_windowWidth(1024), m_windowHeight(512),
      m_enableShadow(true), m_shadowOffsetX(2.0f), m_shadowOffsetY(2.0f),
      m_shadowOpacity(0.7f), m_bgDarkenOpacity(0.3f), m_bgOpacity(0.8f),
      m_backgroundArtMode(0), m_visualizerMode(0), m_logoX(16), m_logoY(16),
      m_logoWidth(64), m_logoHeight(64), m_logoMenuIconSize(24.0f),
      m_logoMenuIconSpacing(40), m_logoMenuIconOffsetX(0),
      m_logoMenuIconOffsetY(0), m_logoMenuScrollDuration(0.5f),
      m_menuLeaveDelay(3.0f), m_logoMenuFontFamily(L"Segoe UI Emoji"),
      m_logoMenuTextColor(L"#FFFFFF"), m_logoMenuTypingFontFamily(L"Consolas"),
      m_logoMenuTypingFontSize(14.0f), m_logoMenuTextOffsetX(0),
      m_logoMenuTextOffsetY(60), m_logoMenuTypingLetterSpacing(0.0f),
      m_baseX(30), m_baseBottomOffset(162), m_artOffsetX(0), m_artOffsetY(0),
      m_artSize(120), m_fallbackArtOpacity(0.5f), m_hoverIconColor(L"#88CCFF"),
      m_hoverFadeOutSpeed(3.0f), m_playingItemColor(L"#FFA500"),
      m_hoverItemColor(L"#FFFF99"),

      m_logoMenuVisualizerFontSize(12.0f), m_logoMenuVisualizerIconOffsetX(12),
      m_logoMenuVisualizerIconOffsetY(7), m_logoMenuLockIconFontSize(14.0f),
      m_logoMenuLockIconOffsetX(12), m_logoMenuLockIconOffsetY(8),
      m_logoMenuDescShadowOffsetX(2.0f), m_logoMenuDescShadowOffsetY(2.0f),

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
  m_controlLeaveDelay = 3.0f;
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

  m_volTooltipText = L"\U0001F5B1\uFE0F\u2195";
  m_volTooltipFontSize = 14.0f;
  m_volTooltipFontFamily = L"Segoe UI Emoji";
  m_volTooltipOffsetY = 10.0f;
  m_volTooltipBgColor = L"#80000000";
  m_volTooltipBgOpacity = 1.0f;
  m_volTooltipTextColor = L"#FFFFFF";
  m_volTooltipWidth = 50.0f;
  m_volTooltipHeight = 26.0f;

  m_skipSeconds = 10.0f;

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
  m_playlistLeaveDelay = 3.0f;
  m_playlistArtistFontSize = 12.0f;
  m_playlistTimeFontSize = 12.0f;

  m_playlistToolbarHeight = 60.0f;
  m_playlistToolbarIconSize = 18.0f;
  m_playlistToolbarIconSpacing = 10.0f;
  m_playlistToolbarTextOffsetY = 30.0f;
  m_playlistToolbarTextFontSize = 12.0f;

  m_pinSubIconOffsetX = 6;
  m_pinSubIconOffsetY = 6;
  m_pinSubIconFontSize = 10.0f;

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
  m_ghGlowOpacity = 0.8f;
  m_ghShadowColor = L"#000000";
  m_ghShadowOpacity = 0.8f;
  m_ghShadowOpacity = 0.8f;
  m_ghKeyColumnOffset = 400.0f;
  m_ghActionColumnOffset = 150.0f;

  m_osdFontFamily = L"Meiryo";
  m_osdFontSize = 48.0f;
  m_osdTextColor = L"#FFFFFF";
  m_osdShadowColor = L"#000000";
  m_osdShadowOpacity = 0.8f;
  m_osdFadeSpeed = 2.0f;
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
  m_lockWindowPosition = GetPrivateProfileIntW(L"Window", L"LockWindowPosition",
                                               0, m_iniFilePath.c_str()) != 0;

  m_isPlaylistPinned =
      GetPrivateProfileIntW(L"Layout_Playlist", L"IsPlaylistPinned", 0,
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

  GetPrivateProfileStringW(L"Layout_Window", L"ControlLeaveDelay", L"3.0", buf,
                           32, m_iniFilePath.c_str());
  try {
    m_controlLeaveDelay = std::stof(buf);
  } catch (...) {
    m_controlLeaveDelay = 3.0f;
  }

  wchar_t colorBuf[256];
  GetPrivateProfileStringW(L"Layout_Window", L"HoverIconColor", L"#88CCFF",
                           colorBuf, 256, m_iniFilePath.c_str());
  m_hoverIconColor = colorBuf;

  GetPrivateProfileStringW(L"Layout_Window", L"HoverFadeOutSpeed", L"3.0", buf,
                           32, m_iniFilePath.c_str());
  try {
    m_hoverFadeOutSpeed = std::stof(buf);
  } catch (...) {
    m_hoverFadeOutSpeed = 3.0f;
  }

  GetPrivateProfileStringW(L"Audio", L"DefaultVolume", L"1.0", buf, 32,
                           m_iniFilePath.c_str());
  try {
    m_defaultVolume = std::stof(buf);
  } catch (...) {
    m_defaultVolume = 1.0f;
  }

  m_shuffleMode = GetPrivateProfileIntW(L"Audio", L"ShuffleMode", 1,
                                        m_iniFilePath.c_str()) != 0;

  GetPrivateProfileStringW(L"Audio", L"SkipSeconds", L"10.0", buf, 32,
                           m_iniFilePath.c_str());
  try {
    m_skipSeconds = std::stof(buf);
  } catch (...) {
    m_skipSeconds = 10.0f;
  }

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

  wchar_t tooltipTextBuf[256];
  GetPrivateProfileStringW(L"Layout_VolumeControl", L"TooltipText", L"",
                           tooltipTextBuf, 256, m_iniFilePath.c_str());
  m_volTooltipText = tooltipTextBuf;
  if (m_volTooltipText.empty() ||
      m_volTooltipText.find(L'?') != std::wstring::npos ||
      m_volTooltipText.find(L"・") != std::wstring::npos) {
    m_volTooltipText = L"\U0001F5B1\uFE0F\u2195";
  }

  GetPrivateProfileStringW(L"Layout_VolumeControl", L"TooltipFontSize", L"14.0",
                           buf, 32, m_iniFilePath.c_str());
  try {
    m_volTooltipFontSize = std::stof(buf);
  } catch (...) {
    m_volTooltipFontSize = 14.0f;
  }

  GetPrivateProfileStringW(L"Layout_VolumeControl", L"TooltipFontFamily",
                           L"Segoe UI Emoji", tooltipTextBuf, 256,
                           m_iniFilePath.c_str());
  m_volTooltipFontFamily = tooltipTextBuf;

  GetPrivateProfileStringW(L"Layout_VolumeControl", L"TooltipOffsetY", L"10.0",
                           buf, 32, m_iniFilePath.c_str());
  try {
    m_volTooltipOffsetY = std::stof(buf);
  } catch (...) {
    m_volTooltipOffsetY = 10.0f;
  }

  GetPrivateProfileStringW(L"Layout_VolumeControl", L"TooltipBgColor",
                           L"#80000000", tooltipTextBuf, 256,
                           m_iniFilePath.c_str());
  m_volTooltipBgColor = tooltipTextBuf;

  GetPrivateProfileStringW(L"Layout_VolumeControl", L"TooltipBgOpacity", L"1.0",
                           buf, 32, m_iniFilePath.c_str());
  try {
    m_volTooltipBgOpacity = std::stof(buf);
  } catch (...) {
    m_volTooltipBgOpacity = 1.0f;
  }

  GetPrivateProfileStringW(L"Layout_VolumeControl", L"TooltipTextColor",
                           L"#FFFFFF", tooltipTextBuf, 256,
                           m_iniFilePath.c_str());
  m_volTooltipTextColor = tooltipTextBuf;

  GetPrivateProfileStringW(L"Layout_VolumeControl", L"TooltipWidth", L"50.0",
                           buf, 32, m_iniFilePath.c_str());
  try {
    m_volTooltipWidth = std::stof(buf);
  } catch (...) {
    m_volTooltipWidth = 50.0f;
  }

  GetPrivateProfileStringW(L"Layout_VolumeControl", L"TooltipHeight", L"26.0",
                           buf, 32, m_iniFilePath.c_str());
  try {
    m_volTooltipHeight = std::stof(buf);
  } catch (...) {
    m_volTooltipHeight = 26.0f;
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

  GetPrivateProfileStringW(L"Layout_LogoMenu", L"MenuLeaveDelay", L"3.0", buf,
                           32, m_iniFilePath.c_str());
  try {
    m_menuLeaveDelay = std::stof(buf);
  } catch (...) {
    m_menuLeaveDelay = 3.0f;
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

  GetPrivateProfileStringW(L"Layout_LogoMenu", L"LockIconFontSize", L"14.0",
                           buf, 32, m_iniFilePath.c_str());
  try {
    m_logoMenuLockIconFontSize = std::stof(buf);
  } catch (...) {
    m_logoMenuLockIconFontSize = 14.0f;
  }

  m_logoMenuLockIconOffsetX = GetPrivateProfileIntW(
      L"Layout_LogoMenu", L"LockIconOffsetX", 12, m_iniFilePath.c_str());

  m_logoMenuLockIconOffsetY = GetPrivateProfileIntW(
      L"Layout_LogoMenu", L"LockIconOffsetY", 8, m_iniFilePath.c_str());

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
      L"Layout_PlaybackControls", L"ButtonSize", 16, m_iniFilePath.c_str());

  wchar_t iconBuf[256];
  GetPrivateProfileStringW(
      L"Layout_PlaybackControls", L"SkipIconPoints",
      L"0.25,-0.5,0.5,-0.5,0.0,0.0,0.5,0.5,0.25,0.5,-0.25,0.0", iconBuf, 256,
      m_iniFilePath.c_str());
  m_skipIconPoints = iconBuf;
  GetPrivateProfileStringW(L"Layout_PlaybackControls", L"SkipTextFontSize",
                           L"10.0", buf, 32, m_iniFilePath.c_str());
  try {
    m_skipTextFontSize = std::stof(buf);
  } catch (...) {
    m_skipTextFontSize = 10.0f;
  }
  GetPrivateProfileStringW(L"Layout_PlaybackControls", L"SkipTextOffsetX",
                           L"0.2", buf, 32, m_iniFilePath.c_str());
  try {
    m_skipTextOffsetX = std::stof(buf);
  } catch (...) {
    m_skipTextOffsetX = 0.2f;
  }
  GetPrivateProfileStringW(L"Layout_PlaybackControls", L"SkipTextOffsetY",
                           L"0.1", buf, 32, m_iniFilePath.c_str());
  try {
    m_skipTextOffsetY = std::stof(buf);
  } catch (...) {
    m_skipTextOffsetY = 0.1f;
  }
  GetPrivateProfileStringW(L"Layout_PlaybackControls", L"SkipTextShadowColor",
                           L"#000000", buf, 32, m_iniFilePath.c_str());
  m_skipTextShadowColor = buf;
  GetPrivateProfileStringW(L"Layout_PlaybackControls", L"SkipTextShadowOpacity",
                           L"0.8", buf, 32, m_iniFilePath.c_str());
  try {
    m_skipTextShadowOpacity = std::stof(buf);
  } catch (...) {
    m_skipTextShadowOpacity = 0.8f;
  }
  GetPrivateProfileStringW(L"Layout_PlaybackControls", L"SkipTextShadowShift",
                           L"1.0", buf, 32, m_iniFilePath.c_str());
  try {
    m_skipTextShadowShift = std::stof(buf);
  } catch (...) {
    m_skipTextShadowShift = 1.0f;
  }

  // Hotfix: Force output to INI file so user can edit them
  WritePrivateProfileStringW(L"Layout_PlaybackControls", L"SkipIconPoints",
                             m_skipIconPoints.c_str(), m_iniFilePath.c_str());
  swprintf_s(buf, L"%.1f", m_skipTextFontSize);
  WritePrivateProfileStringW(L"Layout_PlaybackControls", L"SkipTextFontSize",
                             buf, m_iniFilePath.c_str());
  swprintf_s(buf, L"%.2f", m_skipTextOffsetX);
  WritePrivateProfileStringW(L"Layout_PlaybackControls", L"SkipTextOffsetX",
                             buf, m_iniFilePath.c_str());
  swprintf_s(buf, L"%.2f", m_skipTextOffsetY);
  WritePrivateProfileStringW(L"Layout_PlaybackControls", L"SkipTextOffsetY",
                             buf, m_iniFilePath.c_str());
  WritePrivateProfileStringW(L"Layout_PlaybackControls", L"SkipTextShadowColor",
                             m_skipTextShadowColor.c_str(),
                             m_iniFilePath.c_str());
  swprintf_s(buf, L"%.2f", m_skipTextShadowOpacity);
  WritePrivateProfileStringW(L"Layout_PlaybackControls",
                             L"SkipTextShadowOpacity", buf,
                             m_iniFilePath.c_str());
  swprintf_s(buf, L"%.2f", m_skipTextShadowShift);
  WritePrivateProfileStringW(L"Layout_PlaybackControls", L"SkipTextShadowShift",
                             buf, m_iniFilePath.c_str());

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

  GetPrivateProfileStringW(L"Layout_Playlist", L"PlayingItemColor", L"#FFA500",
                           colorBuf, 256, m_iniFilePath.c_str());
  m_playingItemColor = colorBuf;

  GetPrivateProfileStringW(L"Layout_Playlist", L"HoverItemColor", L"#FFFF99",
                           colorBuf, 256, m_iniFilePath.c_str());
  m_hoverItemColor = colorBuf;

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

  GetPrivateProfileStringW(L"Layout_Playlist", L"PlaylistLeaveDelay", L"3.0",
                           buf, 32, m_iniFilePath.c_str());
  try {
    m_playlistLeaveDelay = std::stof(buf);
  } catch (...) {
    m_playlistLeaveDelay = 3.0f;
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

  GetPrivateProfileStringW(L"Layout_Playlist", L"ToolbarHeight", L"60.0", buf,
                           32, m_iniFilePath.c_str());
  try {
    m_playlistToolbarHeight = std::stof(buf);
  } catch (...) {
    m_playlistToolbarHeight = 60.0f;
  }

  GetPrivateProfileStringW(L"Layout_Playlist", L"ToolbarIconSize", L"18.0", buf,
                           32, m_iniFilePath.c_str());
  try {
    m_playlistToolbarIconSize = std::stof(buf);
  } catch (...) {
    m_playlistToolbarIconSize = 18.0f;
  }

  GetPrivateProfileStringW(L"Layout_Playlist", L"ToolbarIconSpacing", L"10.0",
                           buf, 32, m_iniFilePath.c_str());
  try {
    m_playlistToolbarIconSpacing = std::stof(buf);
  } catch (...) {
    m_playlistToolbarIconSpacing = 10.0f;
  }

  GetPrivateProfileStringW(L"Layout_Playlist", L"ToolbarTextOffsetY", L"30.0",
                           buf, 32, m_iniFilePath.c_str());
  try {
    m_playlistToolbarTextOffsetY = std::stof(buf);
  } catch (...) {
    m_playlistToolbarTextOffsetY = 30.0f;
  }

  GetPrivateProfileStringW(L"Layout_Playlist", L"ToolbarTextFontSize", L"12.0",
                           buf, 32, m_iniFilePath.c_str());
  try {
    m_playlistToolbarTextFontSize = std::stof(buf);
  } catch (...) {
    m_playlistToolbarTextFontSize = 12.0f;
  }

  m_pinSubIconOffsetX = GetPrivateProfileIntW(
      L"Layout_Playlist", L"PinSubIconOffsetX", 6, m_iniFilePath.c_str());
  m_pinSubIconOffsetY = GetPrivateProfileIntW(
      L"Layout_Playlist", L"PinSubIconOffsetY", 6, m_iniFilePath.c_str());
  GetPrivateProfileStringW(L"Layout_Playlist", L"PinSubIconFontSize", L"10.0",
                           buf, 32, m_iniFilePath.c_str());
  try {
    m_pinSubIconFontSize = std::stof(buf);
  } catch (...) {
    m_pinSubIconFontSize = 10.0f;
  }

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

  wchar_t ghBuf[32];
  GetPrivateProfileStringW(L"Layout_GlobalHotkeys", L"FontFamily", L"Meiryo",
                           ghBuf, 32, m_iniFilePath.c_str());
  m_ghFontFamily = ghBuf;
  GetPrivateProfileStringW(L"Layout_GlobalHotkeys", L"FontSize", L"14.0", ghBuf,
                           32, m_iniFilePath.c_str());
  try {
    m_ghFontSize = std::stof(ghBuf);
  } catch (...) {
    m_ghFontSize = 14.0f;
  }
  GetPrivateProfileStringW(L"Layout_GlobalHotkeys", L"LineSpacing", L"20.0",
                           ghBuf, 32, m_iniFilePath.c_str());
  try {
    m_ghLineSpacing = std::stof(ghBuf);
  } catch (...) {
    m_ghLineSpacing = 20.0f;
  }
  GetPrivateProfileStringW(L"Layout_GlobalHotkeys", L"CoreColor", L"#FFFFFF",
                           ghBuf, 32, m_iniFilePath.c_str());
  m_ghCoreColor = ghBuf;
  GetPrivateProfileStringW(L"Layout_GlobalHotkeys", L"GlowColor", L"#00FFFF",
                           ghBuf, 32, m_iniFilePath.c_str());
  m_ghGlowColor = ghBuf;
  GetPrivateProfileStringW(L"Layout_GlobalHotkeys", L"ShadowColor", L"#000000",
                           ghBuf, 32, m_iniFilePath.c_str());
  m_ghShadowColor = ghBuf;
  GetPrivateProfileStringW(L"Layout_GlobalHotkeys", L"ShadowOpacity", L"0.8",
                           buf, 32, m_iniFilePath.c_str());
  try {
    m_ghShadowOpacity = std::stof(buf);
  } catch (...) {
    m_ghShadowOpacity = 0.8f;
  }

  GetPrivateProfileStringW(L"Layout_GlobalHotkeys", L"GlowOpacity", L"0.8", buf,
                           32, m_iniFilePath.c_str());
  try {
    m_ghGlowOpacity = std::stof(buf);
  } catch (...) {
    m_ghGlowOpacity = 0.8f;
  }

  GetPrivateProfileStringW(L"Layout_GlobalHotkeys", L"KeyColumnOffset",
                           L"400.0", buf, 32, m_iniFilePath.c_str());
  try {
    m_ghKeyColumnOffset = std::stof(buf);
  } catch (...) {
    m_ghKeyColumnOffset = 400.0f;
  }

  GetPrivateProfileStringW(L"Layout_GlobalHotkeys", L"ActionColumnOffset",
                           L"150.0", buf, 32, m_iniFilePath.c_str());
  try {
    m_ghActionColumnOffset = std::stof(buf);
  } catch (...) {
    m_ghActionColumnOffset = 150.0f;
  }

  wchar_t osdBuf[256];
  GetPrivateProfileStringW(L"Layout_OSD", L"OsdFontFamily", L"Meiryo", osdBuf,
                           256, m_iniFilePath.c_str());
  m_osdFontFamily = osdBuf;
  GetPrivateProfileStringW(L"Layout_OSD", L"OsdFontSize", L"48.0", buf, 32,
                           m_iniFilePath.c_str());
  try {
    m_osdFontSize = std::stof(buf);
  } catch (...) {
    m_osdFontSize = 48.0f;
  }
  GetPrivateProfileStringW(L"Layout_OSD", L"OsdTextColor", L"#FFFFFF", osdBuf,
                           256, m_iniFilePath.c_str());
  m_osdTextColor = osdBuf;
  GetPrivateProfileStringW(L"Layout_OSD", L"OsdShadowColor", L"#000000", osdBuf,
                           256, m_iniFilePath.c_str());
  m_osdShadowColor = osdBuf;
  GetPrivateProfileStringW(L"Layout_OSD", L"OsdShadowOffsetX", L"2.0", buf, 32,
                           m_iniFilePath.c_str());
  try {
    m_osdShadowOffsetX = std::stof(buf);
  } catch (...) {
    m_osdShadowOffsetX = 2.0f;
  }
  GetPrivateProfileStringW(L"Layout_OSD", L"OsdShadowOffsetY", L"2.0", buf, 32,
                           m_iniFilePath.c_str());
  try {
    m_osdShadowOffsetY = std::stof(buf);
  } catch (...) {
    m_osdShadowOffsetY = 2.0f;
  }
  GetPrivateProfileStringW(L"Layout_OSD", L"OsdShadowOpacity", L"0.8", buf, 32,
                           m_iniFilePath.c_str());
  try {
    m_osdShadowOpacity = std::stof(buf);
  } catch (...) {
    m_osdShadowOpacity = 0.8f;
  }
  GetPrivateProfileStringW(L"Layout_OSD", L"OsdFadeWait", L"1.0", buf, 32,
                           m_iniFilePath.c_str());
  try {
    m_osdFadeWait = std::stof(buf);
  } catch (...) {
    m_osdFadeWait = 1.0f;
  }
  GetPrivateProfileStringW(L"Layout_OSD", L"OsdFadeSpeed", L"2.0", buf, 32,
                           m_iniFilePath.c_str());
  try {
    m_osdFadeSpeed = std::stof(buf);
  } catch (...) {
    m_osdFadeSpeed = 2.0f;
  }

  m_showHotkeys = GetPrivateProfileIntW(L"GlobalHotkeys", L"ShowHotkeys", 0,
                                        m_iniFilePath.c_str()) != 0;
  m_modNextTrack = GetPrivateProfileIntW(
      L"GlobalHotkeys", L"Modifier_NextTrack", 10, m_iniFilePath.c_str());
  m_vkNextTrack = GetPrivateProfileIntW(L"GlobalHotkeys", L"VK_NextTrack", 39,
                                        m_iniFilePath.c_str());
  m_modPrevTrack = GetPrivateProfileIntW(
      L"GlobalHotkeys", L"Modifier_PrevTrack", 10, m_iniFilePath.c_str());
  m_vkPrevTrack = GetPrivateProfileIntW(L"GlobalHotkeys", L"VK_PrevTrack", 37,
                                        m_iniFilePath.c_str());
  m_modPlayPause = GetPrivateProfileIntW(
      L"GlobalHotkeys", L"Modifier_PlayPause", 10, m_iniFilePath.c_str());
  m_vkPlayPause = GetPrivateProfileIntW(L"GlobalHotkeys", L"VK_PlayPause", 38,
                                        m_iniFilePath.c_str());
  m_modStop = GetPrivateProfileIntW(L"GlobalHotkeys", L"Modifier_Stop", 10,
                                    m_iniFilePath.c_str());
  m_vkStop = GetPrivateProfileIntW(L"GlobalHotkeys", L"VK_Stop", 40,
                                   m_iniFilePath.c_str());
  m_modVolUp5 = GetPrivateProfileIntW(L"GlobalHotkeys", L"Modifier_VolUp5", 13,
                                      m_iniFilePath.c_str());
  m_vkVolUp5 = GetPrivateProfileIntW(L"GlobalHotkeys", L"VK_VolUp5", 39,
                                     m_iniFilePath.c_str());
  m_modVolDown5 = GetPrivateProfileIntW(L"GlobalHotkeys", L"Modifier_VolDown5",
                                        13, m_iniFilePath.c_str());
  m_vkVolDown5 = GetPrivateProfileIntW(L"GlobalHotkeys", L"VK_VolDown5", 37,
                                       m_iniFilePath.c_str());
  m_modVolUp25 = GetPrivateProfileIntW(L"GlobalHotkeys", L"Modifier_VolUp25",
                                       13, m_iniFilePath.c_str());
  m_vkVolUp25 = GetPrivateProfileIntW(L"GlobalHotkeys", L"VK_VolUp25", 38,
                                      m_iniFilePath.c_str());
  m_modVolDown25 = GetPrivateProfileIntW(
      L"GlobalHotkeys", L"Modifier_VolDown25", 13, m_iniFilePath.c_str());
  m_vkVolDown25 = GetPrivateProfileIntW(L"GlobalHotkeys", L"VK_VolDown25", 40,
                                        m_iniFilePath.c_str());
  m_modPrevPlaylist = GetPrivateProfileIntW(
      L"GlobalHotkeys", L"Modifier_PrevPlaylist", 10, m_iniFilePath.c_str());
  m_vkPrevPlaylist = GetPrivateProfileIntW(L"GlobalHotkeys", L"VK_PrevPlaylist",
                                           33, m_iniFilePath.c_str());
  m_modNextPlaylist = GetPrivateProfileIntW(
      L"GlobalHotkeys", L"Modifier_NextPlaylist", 10, m_iniFilePath.c_str());
  m_vkNextPlaylist = GetPrivateProfileIntW(L"GlobalHotkeys", L"VK_NextPlaylist",
                                           34, m_iniFilePath.c_str());
  m_modActiveTopMost = GetPrivateProfileIntW(
      L"GlobalHotkeys", L"Modifier_ActiveTopMost", 10, m_iniFilePath.c_str());
  m_vkActiveTopMost = GetPrivateProfileIntW(
      L"GlobalHotkeys", L"VK_ActiveTopMost", 36, m_iniFilePath.c_str());
  m_modActiveBottom = GetPrivateProfileIntW(
      L"GlobalHotkeys", L"Modifier_ActiveBottom", 10, m_iniFilePath.c_str());
  m_vkActiveBottom = GetPrivateProfileIntW(L"GlobalHotkeys", L"VK_ActiveBottom",
                                           35, m_iniFilePath.c_str());
  m_modExitApp = GetPrivateProfileIntW(L"GlobalHotkeys", L"Modifier_ExitApp",
                                       10, m_iniFilePath.c_str());
  m_vkExitApp = GetPrivateProfileIntW(L"GlobalHotkeys", L"VK_ExitApp", 46,
                                      m_iniFilePath.c_str());
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
  WritePrivateProfileStringW(L"Window", L"LockWindowPosition",
                             lock ? L"1" : L"0", m_iniFilePath.c_str());
}

void ConfigManager::SetIsPlaylistPinned(bool pinned) {
  m_isPlaylistPinned = pinned;
  WritePrivateProfileStringW(L"Layout_Playlist", L"IsPlaylistPinned",
                             pinned ? L"1" : L"0", m_iniFilePath.c_str());
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
  WritePrivateProfileStringW(L"Audio", L"ShuffleMode", mode ? L"1" : L"0",
                             m_iniFilePath.c_str());
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

void ConfigManager::SetSkipSeconds(float seconds) {
  m_skipSeconds = seconds;
  wchar_t buf[32];
  swprintf_s(buf, L"%.1f", seconds);
  WritePrivateProfileStringW(L"Audio", L"SkipSeconds", buf,
                             m_iniFilePath.c_str());
}

void ConfigManager::SetShowHotkeys(bool show) {
  m_showHotkeys = show;
  WritePrivateProfileStringW(L"GlobalHotkeys", L"ShowHotkeys",
                             show ? L"1" : L"0", m_iniFilePath.c_str());
}
void ConfigManager::SetNextTrackHotkey(int mod, int vk) {
  m_modNextTrack = mod;
  m_vkNextTrack = vk;
  WritePrivateProfileStringW(L"GlobalHotkeys", L"Modifier_NextTrack",
                             std::to_wstring(mod).c_str(),
                             m_iniFilePath.c_str());
  WritePrivateProfileStringW(L"GlobalHotkeys", L"VK_NextTrack",
                             std::to_wstring(vk).c_str(),
                             m_iniFilePath.c_str());
}
void ConfigManager::SetPrevTrackHotkey(int mod, int vk) {
  m_modPrevTrack = mod;
  m_vkPrevTrack = vk;
  WritePrivateProfileStringW(L"GlobalHotkeys", L"Modifier_PrevTrack",
                             std::to_wstring(mod).c_str(),
                             m_iniFilePath.c_str());
  WritePrivateProfileStringW(L"GlobalHotkeys", L"VK_PrevTrack",
                             std::to_wstring(vk).c_str(),
                             m_iniFilePath.c_str());
}
void ConfigManager::SetPlayPauseHotkey(int mod, int vk) {
  m_modPlayPause = mod;
  m_vkPlayPause = vk;
  WritePrivateProfileStringW(L"GlobalHotkeys", L"Modifier_PlayPause",
                             std::to_wstring(mod).c_str(),
                             m_iniFilePath.c_str());
  WritePrivateProfileStringW(L"GlobalHotkeys", L"VK_PlayPause",
                             std::to_wstring(vk).c_str(),
                             m_iniFilePath.c_str());
}
void ConfigManager::SetStopHotkey(int mod, int vk) {
  m_modStop = mod;
  m_vkStop = vk;
  WritePrivateProfileStringW(L"GlobalHotkeys", L"Modifier_Stop",
                             std::to_wstring(mod).c_str(),
                             m_iniFilePath.c_str());
  WritePrivateProfileStringW(L"GlobalHotkeys", L"VK_Stop",
                             std::to_wstring(vk).c_str(),
                             m_iniFilePath.c_str());
}
void ConfigManager::SetVolUp5Hotkey(int mod, int vk) {
  m_modVolUp5 = mod;
  m_vkVolUp5 = vk;
  WritePrivateProfileStringW(L"GlobalHotkeys", L"Modifier_VolUp5",
                             std::to_wstring(mod).c_str(),
                             m_iniFilePath.c_str());
  WritePrivateProfileStringW(L"GlobalHotkeys", L"VK_VolUp5",
                             std::to_wstring(vk).c_str(),
                             m_iniFilePath.c_str());
}
void ConfigManager::SetVolDown5Hotkey(int mod, int vk) {
  m_modVolDown5 = mod;
  m_vkVolDown5 = vk;
  WritePrivateProfileStringW(L"GlobalHotkeys", L"Modifier_VolDown5",
                             std::to_wstring(mod).c_str(),
                             m_iniFilePath.c_str());
  WritePrivateProfileStringW(L"GlobalHotkeys", L"VK_VolDown5",
                             std::to_wstring(vk).c_str(),
                             m_iniFilePath.c_str());
}
void ConfigManager::SetVolUp25Hotkey(int mod, int vk) {
  m_modVolUp25 = mod;
  m_vkVolUp25 = vk;
  WritePrivateProfileStringW(L"GlobalHotkeys", L"Modifier_VolUp25",
                             std::to_wstring(mod).c_str(),
                             m_iniFilePath.c_str());
  WritePrivateProfileStringW(L"GlobalHotkeys", L"VK_VolUp25",
                             std::to_wstring(vk).c_str(),
                             m_iniFilePath.c_str());
}
void ConfigManager::SetVolDown25Hotkey(int mod, int vk) {
  m_modVolDown25 = mod;
  m_vkVolDown25 = vk;
  WritePrivateProfileStringW(L"GlobalHotkeys", L"Modifier_VolDown25",
                             std::to_wstring(mod).c_str(),
                             m_iniFilePath.c_str());
  WritePrivateProfileStringW(L"GlobalHotkeys", L"VK_VolDown25",
                             std::to_wstring(vk).c_str(),
                             m_iniFilePath.c_str());
}
void ConfigManager::SetNextPlaylistHotkey(int mod, int vk) {
  m_modNextPlaylist = mod;
  m_vkNextPlaylist = vk;
  WritePrivateProfileStringW(L"GlobalHotkeys", L"Modifier_NextPlaylist",
                             std::to_wstring(mod).c_str(),
                             m_iniFilePath.c_str());
  WritePrivateProfileStringW(L"GlobalHotkeys", L"VK_NextPlaylist",
                             std::to_wstring(vk).c_str(),
                             m_iniFilePath.c_str());
}
void ConfigManager::SetPrevPlaylistHotkey(int mod, int vk) {
  m_modPrevPlaylist = mod;
  m_vkPrevPlaylist = vk;
  WritePrivateProfileStringW(L"GlobalHotkeys", L"Modifier_PrevPlaylist",
                             std::to_wstring(mod).c_str(),
                             m_iniFilePath.c_str());
  WritePrivateProfileStringW(L"GlobalHotkeys", L"VK_PrevPlaylist",
                             std::to_wstring(vk).c_str(),
                             m_iniFilePath.c_str());
}
void ConfigManager::SetActiveTopMostHotkey(int mod, int vk) {
  m_modActiveTopMost = mod;
  m_vkActiveTopMost = vk;
  WritePrivateProfileStringW(L"GlobalHotkeys", L"Modifier_ActiveTopMost",
                             std::to_wstring(mod).c_str(),
                             m_iniFilePath.c_str());
  WritePrivateProfileStringW(L"GlobalHotkeys", L"VK_ActiveTopMost",
                             std::to_wstring(vk).c_str(),
                             m_iniFilePath.c_str());
}
void ConfigManager::SetActiveBottomHotkey(int mod, int vk) {
  m_modActiveBottom = mod;
  m_vkActiveBottom = vk;
  WritePrivateProfileStringW(L"GlobalHotkeys", L"Modifier_ActiveBottom",
                             std::to_wstring(mod).c_str(),
                             m_iniFilePath.c_str());
  WritePrivateProfileStringW(L"GlobalHotkeys", L"VK_ActiveBottom",
                             std::to_wstring(vk).c_str(),
                             m_iniFilePath.c_str());
}
void ConfigManager::SetExitAppHotkey(int mod, int vk) {
  m_modExitApp = mod;
  m_vkExitApp = vk;
  WritePrivateProfileStringW(L"GlobalHotkeys", L"Modifier_ExitApp",
                             std::to_wstring(mod).c_str(),
                             m_iniFilePath.c_str());
  WritePrivateProfileStringW(L"GlobalHotkeys", L"VK_ExitApp",
                             std::to_wstring(vk).c_str(),
                             m_iniFilePath.c_str());
}

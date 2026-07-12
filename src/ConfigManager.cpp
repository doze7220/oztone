#include "ConfigManager.h"
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
BgOpacity=0.6
BgDarkenOpacity=0.4
BackgroundArtMode=0
ControlHoverHeight=50.0
HoverIconColor=#ffa500
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
TrackCountOffsetY=-18
TrackCountFontFamily="Courier New"
TrackCountFontSize=18.0
TrackCountLetterSpacing=-3.0
TrackCountShadowOffsetX=1.0
TrackCountShadowOffsetY=1.0
TrackCountShadowOpacity=1.0
TrackCountTextAlignment=1

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
SkipSeconds=10.0

[Visualizer]
VisualizerMode=2
EnablePreScan=1
HighFreqNoiseThreshold=8.0
BandGain0=0.005
BandGain25=0.02
BandGain50=0.05
BandGain75=0.1
BandGain100=1.0

[Visualizer_PrismBeat]
MaxHeightRatio=0.8
PrismLineThickness=1.0
PrismGlow1Thickness=3.0
PrismGlow1Opacity=0.6
PrismGlow2Thickness=8.0
PrismGlow2Opacity=0.2

[Visualizer_HaloDust]
BaseRadiusRatio=0.35
GraphLengthRatio=0.25
HaloGlowOpacity=0.3
HaloGlowThickness=2.0
HaloLaserBaseOpacity=0.15
HaloLaserLengthRatio=0.2
HaloLaserThickness=1.0
HaloLaserSpeed=0.02
HaloLaserSpawnRate=0.15
HaloLaserLifeTime=400.0
HaloParticleBaseOpacity=0.3
HaloParticleSizeRatio=0.02
HaloParticleSpeed=0.015
HaloParticleSpawnRate=0.025
HaloParticleLifeTime=400.0

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
OsdFontFamily=MS Gothic
OsdFontSize=48.0
OsdTextColor=#FFFFFF
OsdShadowColor=#000000
OsdShadowOffsetX=2.0
OsdShadowOffsetY=2.0
OsdShadowOpacity=0.8
OsdFadeWait=1.5
OsdFadeSpeed=1.5
EnableOSD=1

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

int ConfigManager::LoadOrWriteInt(const std::wstring& section, const std::wstring& key, int defaultValue) {
    wchar_t buf[32];
    DWORD len = GetPrivateProfileStringW(section.c_str(), key.c_str(), L"__MISSING__", buf, 32, m_iniFilePath.c_str());
    if (len > 0 && wcscmp(buf, L"__MISSING__") == 0) {
        WritePrivateProfileStringW(section.c_str(), key.c_str(), std::to_wstring(defaultValue).c_str(), m_iniFilePath.c_str());
        return defaultValue;
    }
    try {
        return std::stoi(buf);
    } catch (...) {
        return defaultValue;
    }
}

float ConfigManager::LoadOrWriteFloat(const std::wstring& section, const std::wstring& key, float defaultValue) {
    wchar_t buf[32];
    DWORD len = GetPrivateProfileStringW(section.c_str(), key.c_str(), L"__MISSING__", buf, 32, m_iniFilePath.c_str());
    if (len > 0 && wcscmp(buf, L"__MISSING__") == 0) {
        std::wstringstream wss;
        wss << defaultValue;
        WritePrivateProfileStringW(section.c_str(), key.c_str(), wss.str().c_str(), m_iniFilePath.c_str());
        return defaultValue;
    }
    try {
        return std::stof(buf);
    } catch (...) {
        return defaultValue;
    }
}

std::wstring ConfigManager::LoadOrWriteString(const std::wstring& section, const std::wstring& key, const std::wstring& defaultValue) {
    wchar_t buf[256];
    DWORD len = GetPrivateProfileStringW(section.c_str(), key.c_str(), L"__MISSING__", buf, 256, m_iniFilePath.c_str());
    if (len > 0 && wcscmp(buf, L"__MISSING__") == 0) {
        WritePrivateProfileStringW(section.c_str(), key.c_str(), defaultValue.c_str(), m_iniFilePath.c_str());
        return defaultValue;
    }
    return std::wstring(buf);
}

void ConfigManager::LoadSettings() {
  LoadWindowSettings();
  LoadPlaylistSettings();
  LoadPlaybackSettings();
  LoadLogoMenuSettings();
  LoadVisualizerSettings();
  LoadSystemSettings();
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
    defPlaylistPath = exePath.substr(0, pos) + L"\\playlist.ozl";
  } else {
    defPlaylistPath = L"playlist.ozl";
  }
  WritePrivateProfileStringW(L"Playlist", L"DefaultPlaylistPath",
                             defPlaylistPath.c_str(), m_iniFilePath.c_str());
}


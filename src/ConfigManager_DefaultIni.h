#pragma once

constexpr const wchar_t *DEFAULT_INI_CONTENT = LR"(; OZtone Default Configuration

[Playlist]
DefaultPlaylistPath=playlist.ozl

[Audio]
DefaultVolume=1.0
ShuffleMode=1
SkipSeconds=10.0

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
ShowSeekBar=1
ShowPlaybackControls=1
ShowVolumeControl=1

[UI_Common_Parm]
FocusColor=#FFA500
HoverFadeOutSpeed=2.0
BaseLeaveDelay=3.0
BaseFontFamily=Meiryo
MonoFontFamily="Courier New"
IconFontFamily="Segoe UI Emoji"
OsdFontFamily="MS Gothic"
EnableShadow=1
ShadowColor=#000000
ShadowOffsetX=2.0
ShadowOffsetY=2.0
ShadowOpacity=0.7

[Layout_Window]
BgOpacity=0.6
BgDarkenOpacity=0.4
BackgroundArtMode=0

[Layout_AppLogo]
X=16
Y=16
Width=64
Height=64

[Layout_LogoMenu]
MenuTypingFontSize=14.0
MenuTextColor=#FFFFFF
MenuTextOffsetX=8
MenuTextOffsetY=-18
MenuTypingLetterSpacing=-1.0
MenuIconSize=44.0
MenuIconSpacing=54
MenuIconOffsetX=-24
MenuIconOffsetY=38
MenuScrollDuration=0.5
VisualizerIconFontSize=24.0
VisualizerIconOffsetX=6
VisualizerIconOffsetY=0
LockIconFontSize=20.0
LockIconOffsetX=-2
LockIconOffsetY=-2

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
ArtistOffsetX=130
ArtistOffsetY=60
ArtistFontSize=18.0
TrackCountOffsetX=-82
TrackCountOffsetY=-18
TrackCountFontSize=18.0
TrackCountLetterSpacing=-3.0
TrackCountTextAlignment=1

[Layout_SeekBar]
SeekBarMargin=20.0
Height=8
BottomOffset=25
FgColor=#FFFFFF
FgOpacity=0.4
BgColor=#000000
BgOpacity=0.3
TimeFontSize=20.0
TimeLetterSpacing=-3.0
TimeMarginRight=15.0
TextColor=#FFFFFF
TextOpacity=1.0

[Layout_PlaybackControls]
ControlHoverHeight=50.0
BaseBottomOffset=22
CenterOffsetX=0
ButtonSpacing=55
ButtonSize=30
SkipIconPoints=0.1,-0.5,0.65,-0.5,-0.15,0.0,0.65,0.5,0.1,0.5,-0.65,0.0
SkipTextFontSize=15.0
SkipTextOffsetX=0.04
SkipTextOffsetY=-0.05
SkipTextShadowShift=1.00

[Layout_VolumeControl]
BaseLeftOffset=30
BaseBottomOffset=22
IconSize=30
TextOffsetX=40
TextOffsetY=-12
FontSize=24.0
TextLetterSpacing=-3.0
TooltipIconSize=60
TooltipOffsetY=-5
TooltipBgColor=#000000
TooltipBgOpacity=0.8
TooltipWidth=64
TooltipHeight=50

[Layout_Playlist]
PlaylistPosition=1
IsPlaylistPinned=0
PlaylistHoverWidth=80
PlaylistWidth=250
PlaylistItemOffsetY=45
PlaylistTitleFontSize=16.0
PlaylistTitleOffsetX=20
PlaylistTitleOffsetY=4
PlaylistArtistFontSize=12.0
PlaylistArtistColor=#888888
PlaylistArtistOffsetX=20
PlaylistArtistOffsetY=25
PlaylistTimeFontSize=12.0
PlaylistTimeColor=#888888
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
FontSize=15.0
LineSpacing=20.0
CoreColor=#FFFFFF
GlowColor=#00FFFF
GlowOpacity=0.10
KeyColumnOffset=360.0
ActionColumnOffset=140.0

[Layout_OSD]
OsdFontSize=48.0
OsdTextColor=#FFFFFF
OsdFadeWait=1.5
OsdFadeSpeed=1.5
EnableOSD=1

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

[System]
WatchdogInterval=1.0
WatchdogTimeout=5.0

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

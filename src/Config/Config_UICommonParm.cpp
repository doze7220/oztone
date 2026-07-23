#include "Config_UICommonParm.h"
#include "../ConfigManager.h"
#include "../ConfigManager_DefaultIni.h"

void ConfigManager::LoadSection_UICommonParm(Config_UICommonParm& outConfig) {
    outConfig.FocusColor = LoadOrWriteString(L"UI_Common_Parm", L"FocusColor");
    outConfig.HoverFadeOutSpeed = LoadOrWriteFloat(L"UI_Common_Parm", L"HoverFadeOutSpeed");
    outConfig.BaseLeaveDelay = LoadOrWriteFloat(L"UI_Common_Parm", L"BaseLeaveDelay");
    outConfig.BaseFontFamily = LoadOrWriteString(L"UI_Common_Parm", L"BaseFontFamily");
    outConfig.MonoFontFamily = LoadOrWriteString(L"UI_Common_Parm", L"MonoFontFamily");
    outConfig.IconFontFamily = LoadOrWriteString(L"UI_Common_Parm", L"IconFontFamily");
    outConfig.OsdFontFamily = LoadOrWriteString(L"UI_Common_Parm", L"OsdFontFamily");
    outConfig.EnableShadow = (LoadOrWriteInt(L"UI_Common_Parm", L"EnableShadow") != 0);
    outConfig.ShadowColor = LoadOrWriteString(L"UI_Common_Parm", L"ShadowColor");
    outConfig.ShadowOffsetX = LoadOrWriteFloat(L"UI_Common_Parm", L"ShadowOffsetX");
    outConfig.ShadowOffsetY = LoadOrWriteFloat(L"UI_Common_Parm", L"ShadowOffsetY");
    outConfig.ShadowOpacity = LoadOrWriteFloat(L"UI_Common_Parm", L"ShadowOpacity");
}

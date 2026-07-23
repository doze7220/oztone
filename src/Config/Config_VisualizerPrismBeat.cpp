#include "Config_VisualizerPrismBeat.h"
#include "ConfigManager.h"
#include "ConfigManager_DefaultIni.h"

void ConfigManager::LoadSection_VisualizerPrismBeat(Config_VisualizerPrismBeat& outConfig) {
    outConfig.MaxHeightRatio = LoadOrWriteFloat(L"Visualizer_PrismBeat", L"MaxHeightRatio");
    outConfig.PrismLineThickness = LoadOrWriteFloat(L"Visualizer_PrismBeat", L"PrismLineThickness");
    outConfig.PrismGlow1Thickness = LoadOrWriteFloat(L"Visualizer_PrismBeat", L"PrismGlow1Thickness");
    outConfig.PrismGlow1Opacity = LoadOrWriteFloat(L"Visualizer_PrismBeat", L"PrismGlow1Opacity");
    outConfig.PrismGlow2Thickness = LoadOrWriteFloat(L"Visualizer_PrismBeat", L"PrismGlow2Thickness");
    outConfig.PrismGlow2Opacity = LoadOrWriteFloat(L"Visualizer_PrismBeat", L"PrismGlow2Opacity");
}

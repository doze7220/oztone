#include "Config_Visualizer.h"
#include "../ConfigManager.h"
#include "../ConfigManager_DefaultIni.h"

void ConfigManager::LoadSection_Visualizer(Config_Visualizer& outConfig) {
    outConfig.VisualizerMode = LoadOrWriteInt(L"Visualizer", L"VisualizerMode");
    outConfig.EnablePreScan = LoadOrWriteInt(L"Visualizer", L"EnablePreScan") != 0;
    outConfig.HighFreqNoiseThreshold = LoadOrWriteFloat(L"Visualizer", L"HighFreqNoiseThreshold");
    outConfig.BandGain0 = LoadOrWriteFloat(L"Visualizer", L"BandGain0");
    outConfig.BandGain25 = LoadOrWriteFloat(L"Visualizer", L"BandGain25");
    outConfig.BandGain50 = LoadOrWriteFloat(L"Visualizer", L"BandGain50");
    outConfig.BandGain75 = LoadOrWriteFloat(L"Visualizer", L"BandGain75");
    outConfig.BandGain100 = LoadOrWriteFloat(L"Visualizer", L"BandGain100");
}

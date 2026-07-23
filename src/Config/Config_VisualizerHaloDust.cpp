#include "Config_VisualizerHaloDust.h"
#include "../ConfigManager.h"
#include "../ConfigManager_DefaultIni.h"
#include <windows.h>
#include <string>

void ConfigManager::LoadSection_VisualizerHaloDust(Config_VisualizerHaloDust& outConfig) {
    outConfig.ColorMode = LoadOrWriteInt(L"Visualizer_HaloDust", L"HaloDustColorMode");
    outConfig.FixedColor = LoadOrWriteString(L"Visualizer_HaloDust", L"HaloDustFixedColor");
    outConfig.BaseRadiusRatio = LoadOrWriteFloat(L"Visualizer_HaloDust", L"BaseRadiusRatio");
    outConfig.GraphLengthRatio = LoadOrWriteFloat(L"Visualizer_HaloDust", L"GraphLengthRatio");
    outConfig.HaloLaserBaseOpacity = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloLaserBaseOpacity");
    outConfig.HaloLaserLengthRatio = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloLaserLengthRatio");
    outConfig.HaloLaserThickness = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloLaserThickness");
    outConfig.HaloLaserSpeed = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloLaserSpeed");
    outConfig.HaloLaserSpawnRate = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloLaserSpawnRate");
    outConfig.HaloLaserLifeTime = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloLaserLifeTime");
    outConfig.HaloParticleBaseOpacity = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloParticleBaseOpacity");
    outConfig.HaloParticleSizeRatio = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloParticleSizeRatio");
    outConfig.HaloParticleSpeed = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloParticleSpeed");
    outConfig.HaloParticleSpawnRate = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloParticleSpawnRate");
    outConfig.HaloParticleLifeTime = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloParticleLifeTime");
    outConfig.HaloGlowOpacity = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloGlowOpacity");
    outConfig.HaloGlowThickness = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloGlowThickness");
}

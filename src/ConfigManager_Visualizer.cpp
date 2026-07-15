#include "ConfigManager.h"
#include <string>

void ConfigManager::SetVisualizerMode(int mode) {
  m_visualizerMode = mode;
  WritePrivateProfileStringW(L"Visualizer", L"VisualizerMode",
                             std::to_wstring(mode).c_str(),
                             m_iniFilePath.c_str());
}

void ConfigManager::SetHighFreqNoiseThreshold(float threshold) {
  m_highFreqNoiseThreshold = threshold;
  wchar_t buf[32];
  swprintf_s(buf, L"%.4f", threshold);
  WritePrivateProfileStringW(L"Visualizer", L"HighFreqNoiseThreshold", buf, m_iniFilePath.c_str());
}

void ConfigManager::SetBandGains(float b0, float b25, float b50, float b75, float b100) {
  m_bandGain0 = b0;
  m_bandGain25 = b25;
  m_bandGain50 = b50;
  m_bandGain75 = b75;
  m_bandGain100 = b100;
  wchar_t buf[32];
  swprintf_s(buf, L"%.2f", b0);
  WritePrivateProfileStringW(L"Visualizer", L"BandGain0", buf, m_iniFilePath.c_str());
  swprintf_s(buf, L"%.2f", b25);
  WritePrivateProfileStringW(L"Visualizer", L"BandGain25", buf, m_iniFilePath.c_str());
  swprintf_s(buf, L"%.2f", b50);
  WritePrivateProfileStringW(L"Visualizer", L"BandGain50", buf, m_iniFilePath.c_str());
  swprintf_s(buf, L"%.2f", b75);
  WritePrivateProfileStringW(L"Visualizer", L"BandGain75", buf, m_iniFilePath.c_str());
  swprintf_s(buf, L"%.2f", b100);
  WritePrivateProfileStringW(L"Visualizer", L"BandGain100", buf, m_iniFilePath.c_str());
}

void ConfigManager::SetPrismBeatMaxHeightRatio(float ratio) {
  m_prismBeatMaxHeightRatio = ratio;
  wchar_t buf[32];
  swprintf_s(buf, L"%.2f", ratio);
  WritePrivateProfileStringW(L"Visualizer_PrismBeat", L"MaxHeightRatio", buf, m_iniFilePath.c_str());
}

void ConfigManager::SetHaloDustColorMode(int mode) {
  m_haloDustColorMode = mode;
  WritePrivateProfileStringW(L"Visualizer_HaloDust", L"HaloDustColorMode", std::to_wstring(mode).c_str(), m_iniFilePath.c_str());
}

void ConfigManager::SetHaloDustFixedColor(const std::wstring& color) {
  m_haloDustFixedColor = color;
  WritePrivateProfileStringW(L"Visualizer_HaloDust", L"HaloDustFixedColor", color.c_str(), m_iniFilePath.c_str());
}

void ConfigManager::SetHaloDustBaseRadiusRatio(float ratio) {
  m_haloDustBaseRadiusRatio = ratio;
  wchar_t buf[32];
  swprintf_s(buf, L"%.2f", ratio);
  WritePrivateProfileStringW(L"Visualizer_HaloDust", L"BaseRadiusRatio", buf, m_iniFilePath.c_str());
}

void ConfigManager::SetHaloDustGraphLengthRatio(float ratio) {
  m_haloDustGraphLengthRatio = ratio;
  wchar_t buf[32];
  swprintf_s(buf, L"%.2f", ratio);
  WritePrivateProfileStringW(L"Visualizer_HaloDust", L"GraphLengthRatio", buf, m_iniFilePath.c_str());
}

void ConfigManager::SetHaloLaserBaseOpacity(float opacity) {
  m_haloLaserBaseOpacity = opacity;
  wchar_t buf[32];
  swprintf_s(buf, L"%.2f", opacity);
  WritePrivateProfileStringW(L"Visualizer_HaloDust", L"HaloLaserBaseOpacity", buf, m_iniFilePath.c_str());
}

void ConfigManager::SetHaloLaserLengthRatio(float ratio) {
  m_haloLaserLengthRatio = ratio;
  wchar_t buf[32];
  swprintf_s(buf, L"%.2f", ratio);
  WritePrivateProfileStringW(L"Visualizer_HaloDust", L"HaloLaserLengthRatio", buf, m_iniFilePath.c_str());
}

void ConfigManager::SetHaloLaserThickness(float thickness) {
  m_haloLaserThickness = thickness;
  wchar_t buf[32];
  swprintf_s(buf, L"%.2f", thickness);
  WritePrivateProfileStringW(L"Visualizer_HaloDust", L"HaloLaserThickness", buf, m_iniFilePath.c_str());
}

void ConfigManager::SetHaloLaserSpeed(float speed) {
  m_haloLaserSpeed = speed;
  wchar_t buf[32];
  swprintf_s(buf, L"%.2f", speed);
  WritePrivateProfileStringW(L"Visualizer_HaloDust", L"HaloLaserSpeed", buf, m_iniFilePath.c_str());
}

void ConfigManager::SetHaloLaserSpawnRate(float rate) {
  m_haloLaserSpawnRate = rate;
  wchar_t buf[32];
  swprintf_s(buf, L"%.2f", rate);
  WritePrivateProfileStringW(L"Visualizer_HaloDust", L"HaloLaserSpawnRate", buf, m_iniFilePath.c_str());
}

void ConfigManager::SetHaloLaserLifeTime(float time) {
  m_haloLaserLifeTime = time;
  wchar_t buf[32];
  swprintf_s(buf, L"%.2f", time);
  WritePrivateProfileStringW(L"Visualizer_HaloDust", L"HaloLaserLifeTime", buf, m_iniFilePath.c_str());
}

void ConfigManager::SetHaloParticleBaseOpacity(float opacity) {
  m_haloParticleBaseOpacity = opacity;
  wchar_t buf[32];
  swprintf_s(buf, L"%.2f", opacity);
  WritePrivateProfileStringW(L"Visualizer_HaloDust", L"HaloParticleBaseOpacity", buf, m_iniFilePath.c_str());
}

void ConfigManager::SetHaloParticleSizeRatio(float ratio) {
  m_haloParticleSizeRatio = ratio;
  wchar_t buf[32];
  swprintf_s(buf, L"%.2f", ratio);
  WritePrivateProfileStringW(L"Visualizer_HaloDust", L"HaloParticleSizeRatio", buf, m_iniFilePath.c_str());
}

void ConfigManager::SetHaloParticleSpeed(float speed) {
  m_haloParticleSpeed = speed;
  wchar_t buf[32];
  swprintf_s(buf, L"%.2f", speed);
  WritePrivateProfileStringW(L"Visualizer_HaloDust", L"HaloParticleSpeed", buf, m_iniFilePath.c_str());
}

void ConfigManager::SetHaloParticleSpawnRate(float rate) {
  m_haloParticleSpawnRate = rate;
  wchar_t buf[32];
  swprintf_s(buf, L"%.2f", rate);
  WritePrivateProfileStringW(L"Visualizer_HaloDust", L"HaloParticleSpawnRate", buf, m_iniFilePath.c_str());
}

void ConfigManager::SetHaloParticleLifeTime(float time) {
  m_haloParticleLifeTime = time;
  wchar_t buf[32];
  swprintf_s(buf, L"%.2f", time);
  WritePrivateProfileStringW(L"Visualizer_HaloDust", L"HaloParticleLifeTime", buf, m_iniFilePath.c_str());
}

void ConfigManager::SetHaloGlowOpacity(float opacity) {
  m_haloGlowOpacity = opacity;
  wchar_t buf[32];
  swprintf_s(buf, L"%.2f", opacity);
  WritePrivateProfileStringW(L"Visualizer_HaloDust", L"HaloGlowOpacity", buf, m_iniFilePath.c_str());
}

void ConfigManager::SetHaloGlowThickness(float thickness) {
  m_haloGlowThickness = thickness;
  wchar_t buf[32];
  swprintf_s(buf, L"%.2f", thickness);
  WritePrivateProfileStringW(L"Visualizer_HaloDust", L"HaloGlowThickness", buf, m_iniFilePath.c_str());
}

void ConfigManager::LoadVisualizerSettings() {
  m_visualizerMode = LoadOrWriteInt(L"Visualizer", L"VisualizerMode");
  m_enablePreScan = LoadOrWriteInt(L"Visualizer", L"EnablePreScan") != 0;
  m_highFreqNoiseThreshold = LoadOrWriteFloat(L"Visualizer", L"HighFreqNoiseThreshold");
  m_bandGain0 = LoadOrWriteFloat(L"Visualizer", L"BandGain0");
  m_bandGain25 = LoadOrWriteFloat(L"Visualizer", L"BandGain25");
  m_bandGain50 = LoadOrWriteFloat(L"Visualizer", L"BandGain50");
  m_bandGain75 = LoadOrWriteFloat(L"Visualizer", L"BandGain75");
  m_bandGain100 = LoadOrWriteFloat(L"Visualizer", L"BandGain100");

  m_prismBeatMaxHeightRatio = LoadOrWriteFloat(L"Visualizer_PrismBeat", L"MaxHeightRatio");
  m_prismLineThickness = LoadOrWriteFloat(L"Visualizer_PrismBeat", L"PrismLineThickness");
  m_prismGlow1Thickness = LoadOrWriteFloat(L"Visualizer_PrismBeat", L"PrismGlow1Thickness");
  m_prismGlow1Opacity = LoadOrWriteFloat(L"Visualizer_PrismBeat", L"PrismGlow1Opacity");
  m_prismGlow2Thickness = LoadOrWriteFloat(L"Visualizer_PrismBeat", L"PrismGlow2Thickness");
  m_prismGlow2Opacity = LoadOrWriteFloat(L"Visualizer_PrismBeat", L"PrismGlow2Opacity");

  m_haloDustColorMode = LoadOrWriteInt(L"Visualizer_HaloDust", L"HaloDustColorMode");
  m_haloDustFixedColor = LoadOrWriteString(L"Visualizer_HaloDust", L"HaloDustFixedColor");
  m_haloDustBaseRadiusRatio = LoadOrWriteFloat(L"Visualizer_HaloDust", L"BaseRadiusRatio");
  m_haloDustGraphLengthRatio = LoadOrWriteFloat(L"Visualizer_HaloDust", L"GraphLengthRatio");
  m_haloLaserBaseOpacity = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloLaserBaseOpacity");
  m_haloLaserLengthRatio = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloLaserLengthRatio");
  m_haloLaserThickness = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloLaserThickness");
  m_haloLaserSpeed = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloLaserSpeed");
  m_haloLaserSpawnRate = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloLaserSpawnRate");
  m_haloLaserLifeTime = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloLaserLifeTime");
  m_haloParticleBaseOpacity = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloParticleBaseOpacity");
  m_haloParticleSizeRatio = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloParticleSizeRatio");
  m_haloParticleSpeed = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloParticleSpeed");
  m_haloParticleSpawnRate = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloParticleSpawnRate");
  m_haloParticleLifeTime = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloParticleLifeTime");
  m_haloGlowOpacity = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloGlowOpacity");
  m_haloGlowThickness = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloGlowThickness");
}

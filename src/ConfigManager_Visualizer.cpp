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
  m_visualizerMode = LoadOrWriteInt(L"Visualizer", L"VisualizerMode", 0);
  m_enablePreScan = LoadOrWriteInt(L"Visualizer", L"EnablePreScan", 0) != 0;
  m_highFreqNoiseThreshold = LoadOrWriteFloat(L"Visualizer", L"HighFreqNoiseThreshold", 0.001f);
  m_bandGain0 = LoadOrWriteFloat(L"Visualizer", L"BandGain0", 1.0f);
  m_bandGain25 = LoadOrWriteFloat(L"Visualizer", L"BandGain25", 1.0f);
  m_bandGain50 = LoadOrWriteFloat(L"Visualizer", L"BandGain50", 1.0f);
  m_bandGain75 = LoadOrWriteFloat(L"Visualizer", L"BandGain75", 1.0f);
  m_bandGain100 = LoadOrWriteFloat(L"Visualizer", L"BandGain100", 1.0f);

  m_prismBeatMaxHeightRatio = LoadOrWriteFloat(L"Visualizer_PrismBeat", L"MaxHeightRatio", 0.80f);
  m_prismLineThickness = LoadOrWriteFloat(L"Visualizer_PrismBeat", L"PrismLineThickness", 1.0f);
  m_prismGlow1Thickness = LoadOrWriteFloat(L"Visualizer_PrismBeat", L"PrismGlow1Thickness", 6.0f);
  m_prismGlow1Opacity = LoadOrWriteFloat(L"Visualizer_PrismBeat", L"PrismGlow1Opacity", 0.6f);
  m_prismGlow2Thickness = LoadOrWriteFloat(L"Visualizer_PrismBeat", L"PrismGlow2Thickness", 16.0f);
  m_prismGlow2Opacity = LoadOrWriteFloat(L"Visualizer_PrismBeat", L"PrismGlow2Opacity", 0.2f);

  m_haloDustBaseRadiusRatio = LoadOrWriteFloat(L"Visualizer_HaloDust", L"BaseRadiusRatio", 0.25f);
  m_haloDustGraphLengthRatio = LoadOrWriteFloat(L"Visualizer_HaloDust", L"GraphLengthRatio", 0.30f);
  m_haloLaserBaseOpacity = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloLaserBaseOpacity", 0.3f);
  m_haloLaserLengthRatio = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloLaserLengthRatio", 0.2f);
  m_haloLaserThickness = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloLaserThickness", 2.0f);
  m_haloLaserSpeed = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloLaserSpeed", 1.0f);
  m_haloLaserSpawnRate = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloLaserSpawnRate", 0.1f);
  m_haloLaserLifeTime = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloLaserLifeTime", 30.0f);
  m_haloParticleBaseOpacity = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloParticleBaseOpacity", 0.5f);
  m_haloParticleSizeRatio = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloParticleSizeRatio", 0.05f);
  m_haloParticleSpeed = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloParticleSpeed", 1.0f);
  m_haloParticleSpawnRate = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloParticleSpawnRate", 0.2f);
  m_haloParticleLifeTime = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloParticleLifeTime", 60.0f);
  m_haloGlowOpacity = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloGlowOpacity", 0.6f);
  m_haloGlowThickness = LoadOrWriteFloat(L"Visualizer_HaloDust", L"HaloGlowThickness", 12.0f);
}

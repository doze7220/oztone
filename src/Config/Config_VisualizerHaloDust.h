#pragma once

#include <string>

struct Config_VisualizerHaloDust {
    int ColorMode = 0;
    std::wstring FixedColor;
    float BaseRadiusRatio = 0.0f;
    float GraphLengthRatio = 0.0f;
    float HaloLaserBaseOpacity = 0.0f;
    float HaloLaserLengthRatio = 0.0f;
    float HaloLaserThickness = 0.0f;
    float HaloLaserSpeed = 0.0f;
    float HaloLaserSpawnRate = 0.0f;
    float HaloLaserLifeTime = 0.0f;
    float HaloParticleBaseOpacity = 0.0f;
    float HaloParticleSizeRatio = 0.0f;
    float HaloParticleSpeed = 0.0f;
    float HaloParticleSpawnRate = 0.0f;
    float HaloParticleLifeTime = 0.0f;
    float HaloGlowOpacity = 0.0f;
    float HaloGlowThickness = 0.0f;
};

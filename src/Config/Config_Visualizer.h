#pragma once

struct Config_Visualizer {
    int VisualizerMode = 2;
    bool EnablePreScan = true;
    float HighFreqNoiseThreshold = 8.0f;
    float BandGain0 = 0.005f;
    float BandGain25 = 0.02f;
    float BandGain50 = 0.05f;
    float BandGain75 = 0.1f;
    float BandGain100 = 1.0f;
};

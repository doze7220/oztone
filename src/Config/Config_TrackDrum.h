#pragma once

struct Config_TrackDrum {
    bool EnableTrackDrum = true;
    float MaxDuration = 5.0f;
    float MinSpeed = 0.1f;
    float MaxSpeed = 4.0f;
    float Acceleration = 0.5f;
    float Deceleration = 0.1f;
    float JogDialConfirmDelay = 1.2f;
};

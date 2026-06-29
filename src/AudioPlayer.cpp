#include "AudioPlayer.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

AudioPlayer::AudioPlayer() : m_engine(nullptr), m_initialized(false) {}

AudioPlayer::~AudioPlayer() {
    Uninitialize();
}

bool AudioPlayer::Initialize() {
    if (m_initialized) {
        return true;
    }

    m_engine = new ma_engine;
    ma_result result = ma_engine_init(NULL, m_engine);
    if (result != MA_SUCCESS) {
        delete m_engine;
        m_engine = nullptr;
        return false;
    }

    m_initialized = true;
    return true;
}

void AudioPlayer::Uninitialize() {
    if (m_initialized && m_engine) {
        ma_engine_uninit(m_engine);
        delete m_engine;
        m_engine = nullptr;
        m_initialized = false;
    }
}

bool AudioPlayer::Play(const std::string& filepath) {
    if (!m_initialized || !m_engine) {
        return false;
    }

    ma_result result = ma_engine_play_sound(m_engine, filepath.c_str(), NULL);
    return (result == MA_SUCCESS);
}

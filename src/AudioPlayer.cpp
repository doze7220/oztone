#define MINIAUDIO_IMPLEMENTATION
#include "AudioPlayer.h"

AudioPlayer::AudioPlayer() : m_engine(nullptr), m_initialized(false), m_isSoundLoaded(false), m_cachedLengthSeconds(0.0f) {}

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
    if (m_isSoundLoaded) {
        ma_sound_uninit(&m_sound);
        m_isSoundLoaded = false;
    }

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

    if (m_isSoundLoaded) {
        ma_sound_uninit(&m_sound);
        m_isSoundLoaded = false;
    }

    ma_result result = ma_sound_init_from_file(m_engine, filepath.c_str(), 0, NULL, NULL, &m_sound);
    if (result != MA_SUCCESS) {
        return false;
    }

    m_isSoundLoaded = true;
    
    float length = 0.0f;
    ma_sound_get_length_in_seconds(&m_sound, &length);
    m_cachedLengthSeconds = length;
    
    result = ma_sound_start(&m_sound);
    return (result == MA_SUCCESS);
}

float AudioPlayer::GetPositionSeconds() {
    if (!m_isSoundLoaded) {
        return 0.0f;
    }
    float cursor = 0.0f;
    ma_sound_get_cursor_in_seconds(&m_sound, &cursor);
    return cursor;
}

float AudioPlayer::GetLengthSeconds() {
    if (!m_isSoundLoaded) {
        return 0.0f;
    }
    return m_cachedLengthSeconds;
}

bool AudioPlayer::IsPlaying() {
    if (!m_isSoundLoaded) {
        return false;
    }
    return ma_sound_is_playing(&m_sound) == MA_TRUE;
}

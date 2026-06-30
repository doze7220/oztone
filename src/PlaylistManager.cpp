#include "PlaylistManager.h"

PlaylistManager::PlaylistManager() : m_currentIndex(0) {}

PlaylistManager::~PlaylistManager() {}

void PlaylistManager::Add(const std::string& filepath) {
    m_playlist.push_back(filepath);
}

std::string PlaylistManager::GetCurrentTrack() const {
    if (m_playlist.empty() || m_currentIndex >= m_playlist.size()) {
        return "";
    }
    return m_playlist[m_currentIndex];
}

void PlaylistManager::Advance() {
    if (m_playlist.empty()) return;
    m_currentIndex = (m_currentIndex + 1) % m_playlist.size();
}

std::string PlaylistManager::GetNextTrack() const {
    if (m_playlist.empty()) return "";
    size_t nextIndex = (m_currentIndex + 1) % m_playlist.size();
    return m_playlist[nextIndex];
}

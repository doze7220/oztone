#include "PlaylistManager.h"
#include <fstream>

PlaylistManager::PlaylistManager() : m_currentIndex(0) {}

PlaylistManager::~PlaylistManager() {}

bool PlaylistManager::Add(const std::string& filepath) {
    if (m_playlistSet.find(filepath) != m_playlistSet.end()) {
        return false;
    }
    m_playlist.push_back(filepath);
    m_playlistSet.insert(filepath);
    return true;
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

void PlaylistManager::SaveToFile(const std::string& outPath) const {
    std::ofstream ofs(outPath);
    if (!ofs) return;
    for (const auto& path : m_playlist) {
        ofs << path << "\n";
    }
}

bool PlaylistManager::IsEmpty() const {
    return m_playlist.empty();
}

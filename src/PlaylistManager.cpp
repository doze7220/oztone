#include "PlaylistManager.h"
#include <fstream>
#include <filesystem>
#include <numeric>
#include <algorithm>

PlaylistManager::PlaylistManager() : m_shuffleIndex(0) {
    std::random_device rd;
    m_mt.seed(rd());
}

PlaylistManager::~PlaylistManager() {}

void PlaylistManager::Clear() {
    m_playlist.clear();
    m_playlistSet.clear();
    m_shuffleIndices.clear();
    m_nextShuffleIndices.clear();
    m_shuffleIndex = 0;
}

void PlaylistManager::GenerateShuffleList(std::vector<size_t>& targetList) {
    targetList.clear();
    if (m_playlist.empty()) return;
    targetList.resize(m_playlist.size());
    std::iota(targetList.begin(), targetList.end(), 0);
    std::shuffle(targetList.begin(), targetList.end(), m_mt);
}

void PlaylistManager::InitializeShuffle() {
    GenerateShuffleList(m_shuffleIndices);
    GenerateShuffleList(m_nextShuffleIndices);
    m_shuffleIndex = 0;
}

void PlaylistManager::ShuffleNextLoop() {
    if (m_nextShuffleIndices.size() == m_playlist.size()) {
        std::shuffle(m_nextShuffleIndices.begin(), m_nextShuffleIndices.end(), m_mt);
    } else {
        GenerateShuffleList(m_nextShuffleIndices);
    }
}

bool PlaylistManager::Add(const std::string& filepath) {
    if (m_playlistSet.find(filepath) != m_playlistSet.end()) {
        return false;
    }
    m_playlist.push_back(filepath);
    m_playlistSet.insert(filepath);
    
    size_t newIndex = m_playlist.size() - 1;
    
    if (m_playlist.size() == 1) {
        InitializeShuffle();
    } else {
        if (m_shuffleIndices.size() < m_playlist.size()) {
            m_shuffleIndices.push_back(newIndex);
        }
        if (m_nextShuffleIndices.size() < m_playlist.size()) {
            m_nextShuffleIndices.push_back(newIndex);
        }
    }
    return true;
}

std::string PlaylistManager::GetCurrentTrack() const {
    if (m_playlist.empty() || m_shuffleIndices.empty() || m_shuffleIndex >= m_shuffleIndices.size()) {
        return "";
    }
    return m_playlist[m_shuffleIndices[m_shuffleIndex]];
}

void PlaylistManager::Advance() {
    if (m_playlist.empty() || m_shuffleIndices.empty()) return;
    
    m_shuffleIndex++;
    if (m_shuffleIndex >= m_shuffleIndices.size()) {
        m_shuffleIndices = std::move(m_nextShuffleIndices);
        GenerateShuffleList(m_nextShuffleIndices);
        m_shuffleIndex = 0;
    }
}

void PlaylistManager::Previous() {
    if (m_playlist.empty() || m_shuffleIndices.empty()) return;
    
    if (m_shuffleIndex == 0) {
        m_shuffleIndex = m_shuffleIndices.size() - 1;
    } else {
        m_shuffleIndex--;
    }
}

std::string PlaylistManager::GetNextTrack() const {
    if (m_playlist.empty() || m_shuffleIndices.empty()) return "";
    
    size_t nextIndex = m_shuffleIndex + 1;
    if (nextIndex < m_shuffleIndices.size()) {
        return m_playlist[m_shuffleIndices[nextIndex]];
    } else {
        if (!m_nextShuffleIndices.empty()) {
            return m_playlist[m_nextShuffleIndices[0]];
        }
        return "";
    }
}

void PlaylistManager::SaveToFile(const std::string& outPath) const {
    std::ofstream ofs(outPath);
    if (!ofs) return;
    for (const auto& path : m_playlist) {
        ofs << path << "\n";
    }
}

void PlaylistManager::LoadFromFile(const std::string& inPath) {
    std::ifstream ifs(inPath);
    if (!ifs) return;

    std::string line;
    while (std::getline(ifs, line)) {
        if (line.empty()) continue;

        try {
            std::filesystem::path p(line);
            if (std::filesystem::exists(p) && std::filesystem::is_regular_file(p)) {
                Add(line);
            }
        } catch (...) {
            // パスが無効な場合は無視
        }
    }
    
    if (!m_playlist.empty()) {
        InitializeShuffle();
    }
}

bool PlaylistManager::IsEmpty() const {
    return m_playlist.empty();
}

size_t PlaylistManager::GetCount() const {
    return m_playlist.size();
}

#include "PlaylistManager.h"
#include "ArtFramingDatabase.h"
#include <fstream>
#include <filesystem>
#include <numeric>
#include <algorithm>
#include <windows.h>
#include <sstream>

PlaylistManager::PlaylistManager() : m_shuffleIndex(0) {
    std::random_device rd;
    m_mt.seed(rd());
}

PlaylistManager::~PlaylistManager() {}



void PlaylistManager::Clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
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

void PlaylistManager::RebuildQueue(bool isShuffle) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_shuffleIndices.clear();
    m_nextShuffleIndices.clear();
    if (m_playlist.empty()) return;

    m_shuffleIndices.resize(m_playlist.size());
    std::iota(m_shuffleIndices.begin(), m_shuffleIndices.end(), 0);
    m_nextShuffleIndices = m_shuffleIndices;

    if (isShuffle) {
        std::shuffle(m_shuffleIndices.begin(), m_shuffleIndices.end(), m_mt);
        std::shuffle(m_nextShuffleIndices.begin(), m_nextShuffleIndices.end(), m_mt);
    }
    m_shuffleIndex = 0;
}

void PlaylistManager::WarpToTrack(const std::wstring& filepath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_playlist.empty() || m_shuffleIndices.empty()) return;

    for (size_t i = 0; i < m_shuffleIndices.size(); ++i) {
        if (m_playlist[m_shuffleIndices[i]].filepath == filepath) {
            m_shuffleIndex = i;
            return;
        }
    }
}

void PlaylistManager::InitializeShuffle() {
    std::lock_guard<std::mutex> lock(m_mutex);
    GenerateShuffleList(m_shuffleIndices);
    GenerateShuffleList(m_nextShuffleIndices);
    m_shuffleIndex = 0;
}

void PlaylistManager::ShuffleNextLoop() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_nextShuffleIndices.size() == m_playlist.size()) {
        std::shuffle(m_nextShuffleIndices.begin(), m_nextShuffleIndices.end(), m_mt);
    } else {
        GenerateShuffleList(m_nextShuffleIndices);
    }
}

bool PlaylistManager::Add(const std::wstring& filepath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_playlistSet.find(filepath) != m_playlistSet.end()) {
        return false;
    }
    PlaylistItem item;
    item.filepath = filepath;
    m_playlist.push_back(item);
    m_playlistSet.insert(filepath);
    
    size_t newIndex = m_playlist.size() - 1;
    
    if (m_playlist.size() == 1) {
        GenerateShuffleList(m_shuffleIndices);
        GenerateShuffleList(m_nextShuffleIndices);
        m_shuffleIndex = 0;
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

std::wstring PlaylistManager::GetCurrentTrack() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_playlist.empty() || m_shuffleIndices.empty() || m_shuffleIndex >= m_shuffleIndices.size()) {
        return L"";
    }
    return m_playlist[m_shuffleIndices[m_shuffleIndex]].filepath;
}

void PlaylistManager::Advance() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_playlist.empty() || m_shuffleIndices.empty()) return;
    
    m_shuffleIndex++;
    if (m_shuffleIndex >= m_shuffleIndices.size()) {
        m_shuffleIndices = std::move(m_nextShuffleIndices);
        GenerateShuffleList(m_nextShuffleIndices);
        m_shuffleIndex = 0;
    }
}

void PlaylistManager::RemoveCurrentTrack() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_playlist.empty() || m_shuffleIndices.empty() || m_shuffleIndex >= m_shuffleIndices.size()) return;
    
    size_t removedGlobalIndex = m_shuffleIndices[m_shuffleIndex];
    std::wstring removedFilepath = m_playlist[removedGlobalIndex].filepath;
    
    m_playlist.erase(m_playlist.begin() + removedGlobalIndex);
    m_playlistSet.erase(removedFilepath);
    
    m_shuffleIndices.erase(m_shuffleIndices.begin() + m_shuffleIndex);
    for (size_t& idx : m_shuffleIndices) {
        if (idx > removedGlobalIndex) idx--;
    }
    
    auto it = std::find(m_nextShuffleIndices.begin(), m_nextShuffleIndices.end(), removedGlobalIndex);
    if (it != m_nextShuffleIndices.end()) {
        m_nextShuffleIndices.erase(it);
    }
    for (size_t& idx : m_nextShuffleIndices) {
        if (idx > removedGlobalIndex) idx--;
    }
    
    if (m_shuffleIndex >= m_shuffleIndices.size() && !m_shuffleIndices.empty()) {
        m_shuffleIndices = std::move(m_nextShuffleIndices);
        GenerateShuffleList(m_nextShuffleIndices);
        m_shuffleIndex = 0;
    }
}

void PlaylistManager::Previous() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_playlist.empty() || m_shuffleIndices.empty()) return;
    
    if (m_shuffleIndex == 0) {
        m_shuffleIndex = m_shuffleIndices.size() - 1;
    } else {
        m_shuffleIndex--;
    }
}

std::wstring PlaylistManager::GetNextTrack() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_playlist.empty() || m_shuffleIndices.empty()) return L"";
    
    size_t nextIndex = m_shuffleIndex + 1;
    if (nextIndex < m_shuffleIndices.size()) {
        return m_playlist[m_shuffleIndices[nextIndex]].filepath;
    } else {
        if (!m_nextShuffleIndices.empty()) {
            return m_playlist[m_nextShuffleIndices[0]].filepath;
        }
        return L"";
    }
}

void PlaylistManager::SaveToFile(const std::wstring& outPath) const {
    std::vector<PlaylistItem> playlistCopy;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        playlistCopy = m_playlist;
    }

    std::ofstream ofs(outPath, std::ios::binary);
    if (!ofs) return;
    for (const auto& item : playlistCopy) {
        if (item.filepath.empty()) continue;

        std::wstring wline = item.filepath;

        int size_needed = WideCharToMultiByte(CP_UTF8, 0, wline.c_str(), (int)wline.length(), NULL, 0, NULL, NULL);
        if (size_needed > 0) {
            std::string utf8Str(size_needed, 0);
            WideCharToMultiByte(CP_UTF8, 0, wline.c_str(), (int)wline.length(), &utf8Str[0], size_needed, NULL, NULL);
            ofs << utf8Str << "\n";
        }
    }
}

void PlaylistManager::LoadFromFile(const std::wstring& inPath, ArtFramingDatabase* framingDb) {
    std::ifstream ifs(inPath, std::ios::binary);
    if (!ifs) return;

    std::string line;
    while (std::getline(ifs, line)) {
        if (line.empty()) continue;
        if (line.back() == '\r') line.pop_back();

        int size_needed = MultiByteToWideChar(CP_UTF8, 0, line.c_str(), (int)line.length(), NULL, 0);
        if (size_needed > 0) {
            std::wstring wline(size_needed, 0);
            MultiByteToWideChar(CP_UTF8, 0, line.c_str(), (int)line.length(), &wline[0], size_needed);

            try {
                std::wstringstream wss(wline);
                std::wstring token;
                std::vector<std::wstring> tokens;
                while (std::getline(wss, token, L'\t')) {
                    tokens.push_back(token);
                }

                if (!tokens.empty()) {
                    std::wstring filepath = tokens[0];
                    std::filesystem::path p(filepath);
                    if (std::filesystem::exists(p) && std::filesystem::is_regular_file(p)) {
                        bool added = Add(filepath);
                        // 古いプレイリストファイルのフォーマット検知とマイグレーション
                        if (added && tokens.size() >= 4 && framingDb != nullptr) {
                            try {
                                float offsetX = std::stof(tokens[1]);
                                float offsetY = std::stof(tokens[2]);
                                float scale = std::stof(tokens[3]);
                                framingDb->SetFraming(filepath, offsetX, offsetY, scale);
                            } catch (...) {
                                // 変換エラー時は無視
                            }
                        }
                    }
                }
            } catch (...) {
                // パスが無効な場合は無視
            }
        }
    }
    
    bool isEmpty = false;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        isEmpty = m_playlist.empty();
    }
    if (!isEmpty) {
        InitializeShuffle();
    }
}

bool PlaylistManager::IsEmpty() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_playlist.empty();
}






size_t PlaylistManager::GetCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_playlist.size();
}

size_t PlaylistManager::GetCurrentIndex() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_shuffleIndex;
}

void PlaylistManager::JumpToIndex(size_t index) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_playlist.empty() || m_shuffleIndices.empty()) return;
    if (index >= m_shuffleIndices.size()) return;
    m_shuffleIndex = index;
}

std::vector<std::wstring> PlaylistManager::GetShuffleList() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<std::wstring> list;
    if (m_playlist.empty() || m_shuffleIndices.empty()) {
        return list;
    }
    list.reserve(m_shuffleIndices.size());
    for (size_t idx : m_shuffleIndices) {
        list.push_back(m_playlist[idx].filepath);
    }
    return list;
}




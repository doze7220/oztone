#include "FileManager.h"
#include <tag.h>
#include <mpegfile.h>
#include <audioproperties.h>
#include <id3v2tag.h>
#include <attachedpictureframe.h>
#include <vector>
#include <string>
#include <memory>
#include <windows.h>
#include <filesystem>
#include <algorithm>

namespace {
    std::wstring GetExecutablePath() {
        std::vector<wchar_t> buffer(MAX_PATH);
        DWORD length = GetModuleFileNameW(nullptr, buffer.data(),
                                          static_cast<DWORD>(buffer.size()));
        if (length == 0) {
            return L"";
        }
        return std::wstring(buffer.data(), length);
    }
}

AudioMetadata FileManager::ExtractTextMetadata(const std::wstring& filepath) {
    AudioMetadata metadata;
    if (filepath.empty()) {
        return metadata;
    }

    std::unique_ptr<TagLib::MPEG::File> mpegFile;
    const int maxRetries = 10;
    for (int i = 0; i < maxRetries; ++i) {
        try {
            mpegFile = std::make_unique<TagLib::MPEG::File>(filepath.c_str());
            if (mpegFile->isValid()) {
                break;
            }
        } catch (...) {
            // 例外時は無視してリトライ
        }
        Sleep(50);
    }

    if (!mpegFile || !mpegFile->isValid()) {
        return metadata;
    }

    // テキストメタデータ（タイトル、アーティスト名）の取得
    TagLib::Tag* tag = mpegFile->tag();
    if (tag) {
        metadata.title = tag->title().toWString();
        metadata.artist = tag->artist().toWString();
    }

    // オーディオプロパティから曲の長さを取得
    TagLib::AudioProperties* properties = mpegFile->audioProperties();
    if (properties) {
        metadata.durationSeconds = properties->lengthInSeconds();
    }

    return metadata;
}

std::vector<BYTE> FileManager::ExtractAlbumArtBinary(const std::wstring& filepath) {
    std::vector<BYTE> imageData;
    if (filepath.empty()) {
        return imageData;
    }

    std::unique_ptr<TagLib::MPEG::File> mpegFile;
    const int maxRetries = 10;
    for (int i = 0; i < maxRetries; ++i) {
        try {
            mpegFile = std::make_unique<TagLib::MPEG::File>(filepath.c_str());
            if (mpegFile->isValid()) {
                break;
            }
        } catch (...) {
            // 例外時は無視してリトライ
        }
        Sleep(50);
    }

    if (!mpegFile || !mpegFile->isValid()) {
        return imageData;
    }

    // ID3v2タグからAPICフレーム（アルバムアート画像）を抽出
    TagLib::ID3v2::Tag* tag = mpegFile->ID3v2Tag();
    if (tag) {
        TagLib::ID3v2::FrameList frames = tag->frameListMap()["APIC"];
        if (!frames.isEmpty()) {
            TagLib::ID3v2::AttachedPictureFrame* frame =
                dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(frames.front());
            if (frame) {
                TagLib::ByteVector pictureData = frame->picture();
                if (pictureData.size() > 0) {
                    imageData.assign(pictureData.data(), pictureData.data() + pictureData.size());
                }
            }
        }
    }

    return imageData;
}

std::vector<std::wstring> FileManager::GetAvailablePlaylists(const std::wstring& defaultPlaylistPath) {
    std::vector<std::wstring> playlists;
    try {
        std::filesystem::path currentPath(defaultPlaylistPath);
        std::filesystem::path dir = currentPath.parent_path();
        if (dir.empty()) {
            std::wstring exePath = GetExecutablePath();
            if (!exePath.empty()) {
                size_t pos = exePath.find_last_of(L"\\/");
                if (pos != std::wstring::npos) dir = exePath.substr(0, pos);
            }
            if (dir.empty()) dir = std::filesystem::current_path();
        }
        if (std::filesystem::exists(dir) && std::filesystem::is_directory(dir)) {
            for (const auto& entry : std::filesystem::directory_iterator(dir)) {
                if (entry.is_regular_file()) {
                    std::wstring ext = entry.path().extension().wstring();
                    std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
                    if (ext == L".ozl") playlists.push_back(entry.path().wstring());
                }
            }
        }
    } catch (...) {}
    std::sort(playlists.begin(), playlists.end());
    return playlists;
}

bool FileManager::CheckPlaylistSnapshotChanged(const std::wstring& defaultPlaylistPath, std::vector<std::pair<std::wstring, std::filesystem::file_time_type>>& inOutSnapshot) {
    std::vector<std::pair<std::wstring, std::filesystem::file_time_type>> currentSnapshot;
    bool success = false;
    try {
        std::filesystem::path currentPath(defaultPlaylistPath);
        std::filesystem::path dir = currentPath.parent_path();
        if (dir.empty()) {
            std::wstring exePath = GetExecutablePath();
            if (!exePath.empty()) {
                size_t pos = exePath.find_last_of(L"\\/");
                if (pos != std::wstring::npos) dir = exePath.substr(0, pos);
            }
            if (dir.empty()) dir = std::filesystem::current_path();
        }
        if (std::filesystem::exists(dir) && std::filesystem::is_directory(dir)) {
            for (const auto& entry : std::filesystem::directory_iterator(dir)) {
                if (entry.is_regular_file()) {
                    std::wstring ext = entry.path().extension().wstring();
                    std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
                    if (ext == L".ozl") currentSnapshot.emplace_back(entry.path().wstring(), entry.last_write_time());
                }
            }
            success = true;
        } else { success = true; }
    } catch (...) { return false; }
    if (!success) return false;
    std::sort(currentSnapshot.begin(), currentSnapshot.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
    if (inOutSnapshot.size() != currentSnapshot.size()) { inOutSnapshot = currentSnapshot; return true; }
    for (size_t i = 0; i < currentSnapshot.size(); ++i) {
        if (inOutSnapshot[i].first != currentSnapshot[i].first || inOutSnapshot[i].second != currentSnapshot[i].second) {
            inOutSnapshot = currentSnapshot; return true;
        }
    }
    return false;
}

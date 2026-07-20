#include "TagManager.h"
#include <taglib/tag.h>
#include <taglib/mpeg/mpegfile.h>
#include <taglib/audioproperties.h>
#include <windows.h>

TagManager::TagManager() {}

TagManager::~TagManager() {}

bool TagManager::Load(const std::wstring& filepath) {
    m_title.clear();
    m_artist.clear();
    m_timeString.clear();

    if (filepath.empty()) return false;

    TagLib::MPEG::File mpegFile(filepath.c_str());
    if (!mpegFile.isValid()) {
        return false;
    }

    // 基本タグ情報の取得
    TagLib::Tag* tag = mpegFile.tag();
    if (tag) {
        m_title = tag->title().toWString();
        m_artist = tag->artist().toWString();
    }

    // 音声プロパティから曲の長さを取得
    TagLib::AudioProperties* properties = mpegFile.audioProperties();
    if (properties) {
        int seconds = properties->lengthInSeconds();
        int min = seconds / 60;
        int sec = seconds % 60;
        wchar_t buf[16];
        swprintf(buf, 16, L"%02d:%02d", min, sec);
        m_timeString = buf;
    }

    return true;
}

std::wstring TagManager::GetTitle() const {
    return m_title;
}

std::wstring TagManager::GetArtist() const {
    return m_artist;
}

std::wstring TagManager::GetTimeString() const {
    return m_timeString;
}

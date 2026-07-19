#include "TagManager.h"
#include <taglib/tag.h>
#include <taglib/mpeg/mpegfile.h>
#include <taglib/mpeg/id3v2/id3v2tag.h>
#include <taglib/mpeg/id3v2/id3v2frame.h>
#include <taglib/mpeg/id3v2/frames/attachedpictureframe.h>
#include <taglib/audioproperties.h>
#include <windows.h>

TagManager::TagManager() {}

TagManager::~TagManager() {}

bool TagManager::Load(const std::wstring& filepath) {
    m_title.clear();
    m_artist.clear();
    m_timeString.clear();
    m_albumArtBytes.clear();

    if (filepath.empty()) return false;

    // MPEG::File を一度だけ開き、タイトル・アーティスト・APICを全て取得する。
    // FileRef と MPEG::File を別々に開くと、Windowsのファイルロックにより
    // 2回目のオープンが失敗し、APICフレームを取得できない。
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

    // アルバムアートの取得 (ID3v2 APICフレーム)
    TagLib::ID3v2::Tag* id3v2tag = mpegFile.ID3v2Tag();
    if (id3v2tag) {
        const TagLib::ID3v2::FrameList& frameList = id3v2tag->frameListMap()["APIC"];
        if (!frameList.isEmpty()) {
            auto* picFrame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(frameList.front());
            if (picFrame) {
                TagLib::ByteVector pictureData = picFrame->picture();
                m_albumArtBytes.assign(pictureData.data(), pictureData.data() + pictureData.size());
            }
        }
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

const std::vector<uint8_t>& TagManager::GetAlbumArtBytes() const {
    return m_albumArtBytes;
}

std::vector<BYTE> TagManager::ExtractAlbumArtBinary(const std::wstring& filepath) {
    std::vector<BYTE> result;
    if (filepath.empty()) return result;

    // MPEG::File を一度だけ開き、APICを取得する (Windowsのファイルロック回避)
    TagLib::MPEG::File mpegFile(filepath.c_str());
    if (!mpegFile.isValid()) {
        return result;
    }

    // アルバムアートの取得 (ID3v2 APICフレーム)
    TagLib::ID3v2::Tag* id3v2tag = mpegFile.ID3v2Tag();
    if (id3v2tag) {
        const TagLib::ID3v2::FrameList& frameList = id3v2tag->frameListMap()["APIC"];
        if (!frameList.isEmpty()) {
            auto* picFrame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(frameList.front());
            if (picFrame) {
                TagLib::ByteVector pictureData = picFrame->picture();
                result.assign(pictureData.data(), pictureData.data() + pictureData.size());
            }
        }
    }

    return result;
}

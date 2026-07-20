#include "FileManager.h"
#include <taglib/tag.h>
#include <taglib/mpegfile.h>
#include <taglib/audioproperties.h>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>
#include <vector>
#include <string>

AudioMetadata FileManager::ExtractTextMetadata(const std::wstring& filepath) {
    AudioMetadata metadata;
    if (filepath.empty()) {
        return metadata;
    }

    TagLib::MPEG::File mpegFile(filepath.c_str());
    if (!mpegFile.isValid()) {
        return metadata;
    }

    // テキストメタデータ（タイトル、アーティスト名）の取得
    TagLib::Tag* tag = mpegFile.tag();
    if (tag) {
        metadata.title = tag->title().toWString();
        metadata.artist = tag->artist().toWString();
    }

    // オーディオプロパティから曲の長さを取得
    TagLib::AudioProperties* properties = mpegFile.audioProperties();
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

    TagLib::MPEG::File mpegFile(filepath.c_str());
    if (!mpegFile.isValid()) {
        return imageData;
    }

    // ID3v2タグからAPICフレーム（アルバムアート画像）を抽出
    TagLib::ID3v2::Tag* tag = mpegFile.ID3v2Tag();
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

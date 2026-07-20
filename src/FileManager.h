#pragma once
#include <string>
#include <vector>
#include <windows.h>

/**
 * @brief 音声ファイルから抽出したメタデータを格納する構造体
 */
struct AudioMetadata {
    std::wstring title;
    std::wstring artist;
    int durationSeconds = 0;
    std::vector<BYTE> coverArt;
};

/**
 * @brief ファイルI/Oおよびメタデータ抽出を単一の窓口として引き受ける門番クラス
 * @details 外部に対してTagLib等の外部ライブラリの存在を完全に隠蔽し、Adapterパターンとして機能する。
 */
class FileManager {
public:
    /**
     * @brief 指定されたファイルパスからメタデータを抽出する
     * @param filepath 対象のファイルパス (UTF-16)
     * @return 抽出されたメタデータ構造体。抽出できなかった項目は空または0となる。
     */
    static AudioMetadata ExtractMetadata(const std::wstring& filepath);

private:
    // インスタンス化禁止の静的ユーティリティクラス
    FileManager() = delete;
    ~FileManager() = delete;
    FileManager(const FileManager&) = delete;
    FileManager& operator=(const FileManager&) = delete;
};

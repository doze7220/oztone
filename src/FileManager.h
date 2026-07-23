#pragma once
#include <string>
#include <vector>
#include <windows.h>
#include <filesystem>

/**
 * @brief 音声ファイルから抽出したメタデータを格納する構造体
 */
struct AudioMetadata {
    std::wstring title;
    std::wstring artist;
    int durationSeconds = 0;
};

/**
 * @brief ファイルI/Oおよびメタデータ抽出を単一の窓口として引き受ける門番クラス
 * @details 外部に対してTagLib等の外部ライブラリの存在を完全に隠蔽し、Adapterパターンとして機能する。
 */
class FileManager {
public:
    /**
     * @brief 指定されたファイルパスからテキストのメタデータを抽出する
     * @param filepath 対象のファイルパス (UTF-16)
     * @return 抽出されたテキストメタデータ構造体。抽出できなかった項目は空または0となる。
     */
    static AudioMetadata ExtractTextMetadata(const std::wstring& filepath);

    /**
     * @brief 指定されたファイルパスからアルバムアートのバイナリデータのみを抽出する
     * @param filepath 対象のファイルパス (UTF-16)
     * @return 抽出された画像バイナリ。存在しない、または抽出できなかった場合は空のvectorが返る。
     */
    static std::vector<BYTE> ExtractAlbumArtBinary(const std::wstring& filepath);

    /**
     * @brief 利用可能なプレイリストのファイルパス一覧を取得する
     * @param defaultPlaylistPath 基準となるプレイリストのパス
     * @return プレイリストのファイルパス一覧
     */
    static std::vector<std::wstring> GetAvailablePlaylists(const std::wstring& defaultPlaylistPath);

    /**
     * @brief プレイリストディレクトリ内のファイル構成（スナップショット）に変更があったか確認する
     * @param defaultPlaylistPath 基準となるプレイリストのパス
     * @param inOutSnapshot 現在のスナップショット。変更があれば更新される
     * @return 変更があった場合はtrue
     */
    static bool CheckPlaylistSnapshotChanged(const std::wstring& defaultPlaylistPath, std::vector<std::pair<std::wstring, std::filesystem::file_time_type>>& inOutSnapshot);

private:
    // インスタンス化禁止の静的ユーティリティクラス
    FileManager() = delete;
    ~FileManager() = delete;
    FileManager(const FileManager&) = delete;
    FileManager& operator=(const FileManager&) = delete;
};

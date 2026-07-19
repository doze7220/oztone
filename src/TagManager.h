#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <windows.h>

/**
 * @brief TagLibを用いてMP3ファイル等からメタデータ（曲名、アーティスト名、アルバムアート）を抽出するクラス
 */
class TagManager {
public:
    TagManager();
    ~TagManager();

    /**
     * @brief ファイルを読み込み、タグ情報を解析する
     * @param filepath ファイルパス(UTF-8)
     * @return 成功ならtrue
     */
    bool Load(const std::wstring& filepath);

    /**
     * @brief 曲名を取得する
     */
    std::wstring GetTitle() const;

    /**
     * @brief アーティスト名を取得する
     */
    std::wstring GetArtist() const;

    /**
     * @brief 再生時間文字列（MM:SS形式）を取得する
     */
    std::wstring GetTimeString() const;

    /**
     * @brief アルバムアートのバイナリデータを取得する
     */
    const std::vector<uint8_t>& GetAlbumArtBytes() const;

    /**
     * @brief 曲ファイルから直接アルバムアートの生バイナリを抽出して返す
     * @param filepath ファイルパス
     * @return 生バイナリデータ(見つからない場合は空)
     */
    static std::vector<BYTE> ExtractAlbumArtBinary(const std::wstring& filepath);

private:
    std::wstring m_title;
    std::wstring m_artist;
    std::wstring m_timeString;
    std::vector<uint8_t> m_albumArtBytes;
};

#pragma once
#include <string>
#include <vector>
#include <set>

/**
 * @brief 再生待ちキュー（プレイリスト）を管理するクラス
 */
class PlaylistManager {
public:
    PlaylistManager();
    ~PlaylistManager();

    /**
     * @brief プレイリストに曲のファイルパスを追加する
     * @param filepath 曲のファイルパス（絶対パス推奨）
     * @return 追加に成功した（重複しなかった）場合はtrue、既に存在した場合はfalse
     */
    bool Add(const std::string& filepath);

    /**
     * @brief 現在の曲のファイルパスを取得する
     * @return 現在の曲のファイルパス。リストが空の場合は空文字を返す。
     */
    std::string GetCurrentTrack() const;

    /**
     * @brief キューを次の曲へ進める（末尾の場合は先頭に戻る）
     */
    void Advance();

    /**
     * @brief 次に再生される曲のファイルパスを取得する
     * @return 次の曲のファイルパス。リストが空の場合は空文字を返す。
     */
    std::string GetNextTrack() const;

    /**
     * @brief プレイリストをファイルに保存する
     * @param outPath 保存先のファイルパス
     */
    void SaveToFile(const std::string& outPath) const;

    /**
     * @brief ファイルからプレイリストを読み込む
     * @param inPath 読み込み元のファイルパス
     */
    void LoadFromFile(const std::string& inPath);

    /**
     * @brief プレイリストが空かどうかを返す
     * @return 空の場合はtrue
     */
    bool IsEmpty() const;

private:
    std::vector<std::string> m_playlist;
    std::set<std::string> m_playlistSet;
    size_t m_currentIndex;
};

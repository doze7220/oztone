#pragma once
#include <string>
#include <vector>
#include <set>
#include <random>

/**
 * @brief 再生待ちキュー（プレイリスト）を管理するクラス
 */
class PlaylistManager {
public:
    PlaylistManager();
    ~PlaylistManager();

    /**
     * @brief プレイリストの内部キューおよびシャッフルインデックスを全て空にする
     */
    void Clear();

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
     * @brief キューを次の曲へ進める（末尾の場合は次周のシャッフルリストに移行する）
     */
    void Advance();

    /**
     * @brief キューを前の曲へ戻す（先頭の場合はリストの末尾へ戻る）
     */
    void Previous();

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

    bool IsEmpty() const;

    /**
     * @brief プレイリストの曲数を取得する
     * @return 曲数
     */
    size_t GetCount() const;

    /**
     * @brief 現在および次周のシャッフルリストを初期化・生成する
     */
    void InitializeShuffle();

    /**
     * @brief 次周のシャッフルリストのみをシャッフルし直す
     */
    void ShuffleNextLoop();

private:
    void GenerateShuffleList(std::vector<size_t>& targetList);

    std::vector<std::string> m_playlist;
    std::set<std::string> m_playlistSet;
    
    std::vector<size_t> m_shuffleIndices;
    std::vector<size_t> m_nextShuffleIndices;
    size_t m_shuffleIndex;

    std::mt19937 m_mt;
};

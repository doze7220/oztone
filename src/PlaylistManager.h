#pragma once
#include <string>
#include <vector>
#include <set>
#include <random>
#include <mutex>

/**
 * @brief プレイリストの1要素を表す構造体（純粋な曲順とフレーミング情報のみ）
 */
struct PlaylistItem {
    std::wstring filepath;
    float artOffsetX = 0.0f;
    float artOffsetY = 0.0f;
    float artScale = 1.0f;
};

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
     * @brief 現在のプレイリスト構成に基づいてシャッフル（または連番）のキューを再構築する
     * @param isShuffle trueならシャッフル、falseなら連番で再構築する
     */
    void RebuildQueue(bool isShuffle);

    /**
     * @brief 指定されたファイルパスの曲が次に再生されるようインデックスを移動（ワープ）させる
     * @param filepath ワープ先の曲のファイルパス
     */
    void WarpToTrack(const std::wstring& filepath);

    /**
     * @brief プレイリストに曲のファイルパスを追加する
     * @param filepath 曲のファイルパス（絶対パス推奨）
     * @return 追加に成功した（重複しなかった）場合はtrue、既に存在した場合はfalse
     */
    bool Add(const std::wstring& filepath);

    /**
     * @brief 現在の曲のファイルパスを取得する
     * @return 現在の曲のファイルパス。リストが空の場合は空文字を返す。
     */
    std::wstring GetCurrentTrack() const;

    /**
     * @brief キューを次の曲へ進める（末尾の場合は次周のシャッフルリストに移行する）
     */
    void Advance();

    /**
     * @brief 現在の曲をプレイリストから削除する
     */
    void RemoveCurrentTrack();

    /**
     * @brief キューを前の曲へ戻す（先頭の場合はリストの末尾へ戻る）
     */
    void Previous();

    /**
     * @brief 次に再生される曲のファイルパスを取得する
     * @return 次の曲のファイルパス。リストが空の場合は空文字を返す。
     */
    std::wstring GetNextTrack() const;

    /**
     * @brief プレイリストをファイルに保存する
     * @param outPath 保存先のファイルパス
     */
    void SaveToFile(const std::wstring& outPath) const;

    /**
     * @brief ファイルからプレイリストを読み込む
     * @param inPath 読み込み元のファイルパス
     */
    void LoadFromFile(const std::wstring& inPath);

    /**
     * @brief 特定のファイルパスの背景アートフレーミング情報を更新する
     * @param filepath 更新対象のファイルパス
     * @param offsetX Xオフセット
     * @param offsetY Yオフセット
     * @param scale スケール
     */
    void UpdateArtFraming(const std::wstring& filepath, float offsetX, float offsetY, float scale);

    /**
     * @brief 特定のファイルパスの背景アートフレーミング情報を取得する
     */
    void GetArtFraming(const std::wstring& filepath, float& offsetX, float& offsetY, float& scale) const;

    bool IsEmpty() const;

    /**
     * @brief プレイリストの曲数を取得する
     * @return 曲数
     */
    size_t GetCount() const;

    /**
     * @brief 現在のシャッフルインデックス（自分が今何曲目にいるか）を取得する
     * @return 現在のインデックス（0始まり）
     */
    size_t GetCurrentIndex() const;

    /**
     * @brief 指定したシャッフルインデックスへ直接ジャンプする
     * @param index ジャンプ先のインデックス
     */
    void JumpToIndex(size_t index);

    /**
     * @brief 現在のシャッフルリスト全体（次に再生される順番）のファイルパス一覧を取得する
     * @return ファイルパスの配列
     */
    std::vector<std::wstring> GetShuffleList() const;


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

    std::vector<PlaylistItem> m_playlist;
    std::set<std::wstring> m_playlistSet;
    
    std::vector<size_t> m_shuffleIndices;
    std::vector<size_t> m_nextShuffleIndices;
    size_t m_shuffleIndex;

    std::mt19937 m_mt;
    mutable std::mutex m_mutex;
};

#pragma once
#include <string>
#include <vector>
#include <set>
#include <random>
#include <mutex>

struct TrackMetadata {
    std::wstring filepath;
    std::wstring title;
    std::wstring artist;
    std::wstring timeString; // 例: "03:45"
    bool isLoaded = false;   // 解析済みかどうか
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
     * @brief 特定のファイルパスのメタデータを更新し、解析済み(isLoaded = true)にする
     * @param filepath 更新対象のファイルパス
     * @param title タイトル
     * @param artist アーティスト
     * @param timeString 再生時間文字列
     */
    void UpdateMetadata(const std::wstring& filepath, const std::wstring& title, const std::wstring& artist, const std::wstring& timeString);

    /**
     * @brief 特定のファイルパスが解析済みかどうかを確認する
     * @param filepath 確認対象のファイルパス
     * @return 解析済みならtrue、未解析または存在しない場合はfalse
     */
    bool IsTrackLoaded(const std::wstring& filepath) const;

    /**
     * @brief 特定のファイルパスのメタデータを取得する
     * @param filepath 取得対象のファイルパス
     * @param outMeta 取得結果を格納する構造体
     * @return 存在すればtrue、しなければfalse
     */
    bool GetTrackMetadata(const std::wstring& filepath, TrackMetadata& outMeta) const;


    /**
     * @brief 未解析(isLoaded == false)のファイルパス一覧を取得する
     * @return ファイルパスの配列
     */
    std::vector<std::wstring> GetUnparsedTracks() const;

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
     * @brief 現在のシャッフルリスト全体（次に再生される順番）のメタデータ一覧を取得する
     * @return メタデータの配列
     */
    std::vector<TrackMetadata> GetShuffleMetadataList() const;


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

    std::vector<TrackMetadata> m_playlist;
    std::set<std::wstring> m_playlistSet;
    
    std::vector<size_t> m_shuffleIndices;
    std::vector<size_t> m_nextShuffleIndices;
    size_t m_shuffleIndex;

    std::mt19937 m_mt;
    mutable std::mutex m_mutex;
};

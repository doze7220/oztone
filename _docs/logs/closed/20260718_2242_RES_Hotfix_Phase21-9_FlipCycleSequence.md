# RES:実装計画・作業レポート Phase 21-9: フリップサイクル処理順序の修正と処理集約

## 1. 実装目的
現在、曲切り替え時に「古い方へアートが表示される」「曲開始時に数秒間アートが消える」という不具合が発生している。
原因は、各呼び出し元（`Application_Playback.cpp`など）で画像を同期的にロードして `Renderer::SetAlbumArt` でセットした**後**に、`Application::PlayCurrentTrack` を呼んで内部でフリップ（`SetDrumTarget`）を行っているため、「フリップ前のカレントスロット（これから過去になるスロット）」に新しい画像が設定され、フリップ後の新カレントスロットは空（`nullptr`）のままになってしまうことにある。
この順序の逆転を解消するため、あちこちに散らばる画像・フレーミングセット処理を削除し、`Application::PlayCurrentTrack` 内部に集約（カプセル化）することで、必ず「フリップしてから新スロットへセットする」という正しいライフサイクルを確立する。

## 2. アーキテクチャ設計
### 要件1: `PlayCurrentTrack` 内部への画像セットの集約とフリップ順序の修正
    - `Application::PlayCurrentTrack(int relativeDistance)` において、オーディオ再生が成功した場合、最初に `m_renderer.SetDrumTarget(relativeDistance)` を呼び出してフリップを確定させる。
    - その直後に、先読みデータがある場合はそれを、ない場合は同期的にタグをロードして、新しいカレントスロットに対して `SetAlbumArt` で画像をセットする。
    - 続けて `SetBackgroundFraming` でフレーミング情報をセットする。

### 要件2: 各 Application モジュールからの冗長なロード処理のパージ
    - `Application_Playback.cpp` (NEXT/PREV処理など)
    - `Application_Render.cpp` (楽曲の自動遷移処理など)
    - `Application_Playlist.cpp` (プレイリストからの再生・削除後など)
    - `Application_Initialize.cpp` (起動時のレジューム再生など)
    - `Application_FileDrop.cpp` (ファイルドロップ時など)
    これらに散在している、`PlayCurrentTrack` 呼び出し前の冗長な `m_tagManager.Load(track)`、`SetAlbumArt`、`SetBackgroundFraming` 等のコードを完全に削除し、`PlayCurrentTrack` の呼び出しのみに純化する。

## 3. 実装タスクリスト
- [x] タスク1: `Application::PlayCurrentTrack` の実装改修
    - `Application_Playback.cpp` にある `PlayCurrentTrack` 関数を改修し、フリップ後に画像・フレーミングセットを行うロジックを実装。
- [x] タスク2: 呼び出し元からの冗長コード削除
    - `Application_Playback.cpp` (HandleMediaCommand内)
    - `Application_Render.cpp` (自動曲送り内)
    - `Application_Playlist.cpp` (アイテムクリック・削除等の各種アクション内)
    - `Application_Initialize.cpp` (レジューム再生内)
    - `Application_FileDrop.cpp` (ドロップ再生内)

## 4. 詳細作業内容
### タスク1: `Application::PlayCurrentTrack` の実装改修
    - `Application_Playback.cpp` の `PlayCurrentTrack` メソッドを改修し、`m_renderer.SetDrumTarget(relativeDistance)` 呼び出し直後に、画像のセット (`SetAlbumArt`) とフレーミング情報のセット (`SetBackgroundFraming`) を行うようロジックを集約・カプセル化しました。
    - 順方向への曲送り (`relativeDistance == 1`) 時はプリフェッチ画像を利用し、それ以外は同期ロードする処理を組み込みました。

### タスク2: 呼び出し元からの冗長コード削除
    - 以下の各モジュールから、`PlayCurrentTrack` 呼び出し前に行われていた冗長な `m_tagManager.Load(track)`、`SetAlbumArt`、および `SetBackgroundFraming` の一連の処理を完全に削除しました。
        - `Application_Playback.cpp` (HandleMediaCommand内)
        - `Application_Render.cpp` (自動曲送り内)
        - `Application_Playlist.cpp` (アイテム削除、アイテムクリック、プレイリスト切り替え内)
        - `Application_Initialize.cpp` (起動時のレジューム再生内)
        - `Application_FileDrop.cpp` (ファイルドロップによる再生内)
    - これにより、各モジュールからの呼び出しは純粋な `PlayCurrentTrack(distance)` のみに集約され、タグ情報のロードや画像のセットは常にフリップサイクル後に行われる正常な順序が保証されました。

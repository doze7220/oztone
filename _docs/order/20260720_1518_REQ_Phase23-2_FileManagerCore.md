##### 作業指示書 REQ: Phase 23-2: FileManagerの新設とTagManagerの吸収 (計画立案)
以下のプロジェクトルールと開発資料を熟読すること。
*  D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  D:\ozlab\oztone\PROJECT_ARCHITECTURE.md

###### 【作業手順（厳守事項）】
本プロンプトでは **絶対にコードの修正（ファイルの書き換え）を行わない** こと。計画立案のみに留めること。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 以下の【実装要件】を満たすための高度なアーキテクチャ設計と実装計画を立案し、AIがハルシネーションを起こさない適切なスコープを持つ細かなタスクリストを構築すること。
3. 計画を出力する前に内部で自己監査を行い、ルール・開発資料の責務違反（Managerの巨大化など）がないか確認して計画を純化すること。
4. 監査を通過した純度100%の計画書を、PROJECT_CONSTITUTION.md の「3.3. 実装計画、兼作業レポート (RES) 標準出力フォーマット」に完全に準拠した作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase23-2_FileManager_and_TagManager.md）として新規作成（出力）すること。
5. チャットにて「計画書の作成が完了しました。タスクを実行するための新しいチャットへ移行してください」と報告すること。

###### 【実装要件】
本フェーズでは、ファイルI/Oおよびメタデータ抽出を一手に引き受ける最強の門番クラス `FileManager` を新規に構築し、既存の `TagManager` を解体・吸収する。
*   **要件1: FileManager クラスの新設とTagLibの統合**
    *   `src/FileManager.h` および `src/FileManager.cpp` を新規作成する。
    *   ファイルパス (`std::wstring`) を受け取り、TagLibを用いて必要な情報（曲名、アーティスト名、APIC画像バイナリ、曲の長さ等）だけを抽出して構造体等で返すメソッドを実装する（ファイルを丸ごとメモリに読み込む愚策は避けること）。
*   **要件2: TagManager の解体と吸収**
    *   既存の `src/TagManager.h` および `src/TagManager.cpp` を削除（または無効化）し、その責務と実装を `FileManager` の内部にカプセル化（Adapterパターン）して移管する。
    *   外部（Application等）は TagLib の存在を知らず、`FileManager` に「メタデータを頂戴」「画像バイナリを頂戴」とだけ依頼するアーキテクチャとする。
*   **要件3: Applicationクラス等への配線と置き換え**
    *   `Application.h` 等で `TagManager` をインスタンス化していた箇所を `FileManager` に置き換え、既存のメタデータ抽出処理が正しく動作するように結線する。
    *   `CMakeLists.txt` を更新し、新設したファイルを追加、削除したファイルを除外してビルドが通る状態にする。

###### 【タスクごとの終了後作業】
1. 作業レポート（D:\ozlab\oztone\_docs\logs\YYYYMMDD_HHMM_RES_Phase23-2_FileManager_and_TagManager.md）に、詳細作業内容を記載する（タスクリストに含める）こと。

###### 【全タスク終了後の作業】
1. D:\ozlab\oztone\PROJECT_ARCHITECTURE.md を確認し、作業内容が記載に影響のある場合は修正を行う（タスクリストに含める）こと。

###### 【実装作業での絶対遵守ルール (Constraints)】
*   **スコープの厳守**: 今回のフェーズでは `FileManager` の実装と `TagManager` の吸収、それに伴う既存のメタデータ取得処理の置き換えのみを行う。`AudioManager` や `ThumbnailManager` の新設、AudioPlayerのインメモリ化など、後続フェーズのタスクには絶対に触れないこと。

-------------------------------------------------------------------------------

##### 作業指示書 REQ: Phase 23-2 タスク1: FileManagerのインターフェース設計と作成 (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  実装計画書:D:\ozlab\oztone\_docs\logs\20260720_1558_RES_Phase23-2_FileManager_and_TagManager.md

###### 【作業手順（厳守事項）】
本プロンプトはPhase 23-2 タスク1: FileManagerのインターフェース設計と作成のHotfixである。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 実装計画書（20260720_1558_RES_Phase23-2_FileManager_and_TagManager.md）を読み、現在の状況と今回の修正スコープを確認すること。
3. 上記を確認した後、以下の【実装要件】に従って **「タスク1のみ」** の実装を開始し、ソースコードの修正を実行すること。絶対にタスク2以降をフライングで実行しないこと。
4. コード修正が完全に終わった後、既存の作業レポート（20260720_1558_RES_Phase23-2_FileManager_and_TagManager.md）の末尾に、既存実装Hotfix追記対応時テンプレート（D:\ozlab\oztone\_docs\RES(AddHotfix)_template.md）を用いて作業の詳細を追記すること。
5. 開発資料（PROJECT_ARCHITECTURE.md）を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
6. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
*  `src/FileManager.h` を新規作成する。
*  クラス外（Application等）から `TagLib` への依存を完全に隠蔽する（Adapterパターン）ため、このヘッダファイル内では絶対に `<taglib/...>` のような外部ライブラリのインクルードを行わないこと。
*  ファイルパス (`std::wstring`) を受け取り、「曲名」「アーティスト名」「曲の長さ(秒)」「アルバムアートのバイナリデータ(`std::vector<BYTE>` 等)」といった必要なメタデータのみを返す（あるいは専用の構造体にまとめて返す）美しく無駄のないインターフェース（メソッド）を設計・宣言すること。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守** : 本タスクの対象は `src/FileManager.h` の新規作成（インターフェースの設計・定義）のみである。`.cpp` の実装や他の既存クラスの修正、ビルド環境の更新などは次以降のタスクで行うため、絶対に実行しないこと。

-------------------------------------------------------------------------------

##### 作業指示書 REQ: Phase 23-2 Hotfix / FileManagerインターフェースの責務分離 (実装実行)
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  実装計画書:D:\ozlab\oztone\_docs\logs\20260720_1558_RES_Phase23-2_FileManager_and_TagManager.md

###### 【作業手順（厳守事項）】
本プロンプトは Phase 23-2 タスク1 で作成された `FileManager.h` のインターフェース責務分離に関するHotfixである。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 実装計画書（20260720_1558_RES_Phase23-2_FileManager_and_TagManager.md）を読み、現在の状況と今回の修正スコープを確認すること。
3. 上記を確認した後、以下の【実装要件】に従って実装を開始し、ソースコードの修正を実行すること。
4. コード修正が完全に終わった後、既存の作業レポート（20260720_1558_RES_Phase23-2_FileManager_and_TagManager.md）の末尾に、既存実装Hotfix追記対応時テンプレート（D:\ozlab\oztone\_docs\RES(AddHotfix)_template.md）を用いて作業の詳細を追記すること。
5. 開発資料（PROJECT_ARCHITECTURE.md）を確認し、作業内容が記載に影響のある場合は資料の修正を行うこと。
6. チャットにて「実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
*   **修正の理由と背景（なぜ分ける必要があるのか）**: 
    プレイリストに大量の曲が追加された際、バックグラウンドの解析スレッドは「全曲のテキスト情報（曲名や長さ）」を一気に読み込む必要がある。もし `AudioMetadata` 構造体に画像バイナリが含まれたままだと、100曲追加された瞬間に100曲分の重い画像データまで無駄にメモリに吸い上げてしまい、アプリのフリーズやメモリパンクといった致命的なパフォーマンス低下を引き起こす。画像バイナリは必要な時に必要な曲だけ（オンデマンドで）取得すべきであるため、テキスト抽出と画像抽出の責務（メソッド）を完全に分離する必要がある。
*   **`src/FileManager.h` の修正**:
    *   `AudioMetadata` 構造体から画像バイナリのフィールド (`std::vector<BYTE>` 等) を削除し、軽く高速なテキスト情報（曲名、アーティスト名、曲の長さ等）だけを保持する軽量な構造体に変更すること。
    *   既存の抽出メソッドを、テキスト情報のみを返すメソッド（例: `static AudioMetadata ExtractTextMetadata(const std::wstring& filepath);`）に名称変更および修正すること。
    *   重い画像バイナリだけをピンポイントで吸い上げて返す専用のメソッド（例: `static std::vector<BYTE> ExtractAlbumArtBinary(const std::wstring& filepath);`）を新たに宣言すること。

#### 【絶対遵守ルール (Constraints)】
*   **TagLib依存の隠蔽維持**: メソッドを分割しても、クラス外（Application等）から `TagLib` への依存を隠蔽する（Adapterパターン）という絶対ルールに変更はない。ヘッダファイル内で `<taglib/...>` のようなインクルードは絶対に行わないこと。
*   **スコープの厳守**: 今回は `src/FileManager.h` の修正のみを行う。`.cpp` の実装は次タスクで行うため触れないこと。

-------------------------------------------------------------------------------

##### 作業指示書 REQ: Phase 23-2 Task 2 : FileManagerの実装（TagManagerロジックの移植）
*  ルール: D:\ozlab\oztone\PROJECT_CONSTITUTION.md
*  開発資料:D:\ozlab\oztone\PROJECT_ARCHITECTURE.md
*  実装計画書:D:\ozlab\oztone\_docs\logs\20260720_1558_RES_Phase23-2_FileManager_and_TagManager.md

###### 【作業手順（厳守事項）】
本プロンプトはPhase 23-2 Task 2: FileManagerの実装（TagManagerロジックの移植）である。必ず以下の順序で作業を行うこと。
1. ルール（PROJECT_CONSTITUTION.md）および開発資料（PROJECT_ARCHITECTURE.md）を熟読・把握すること。
2. 実装計画書（20260720_1558_RES_Phase23-2_FileManager_and_TagManager.md）を読み、今回の自分のスコープが「タスク2」のみであることを確認すること。
3. 上記を確認した後、以下の【実装要件】に従って **「タスク2のみ」** の実装を開始し、ソースコードの修正を実行すること。絶対にタスク3以降をフライングで実行しないこと。
4. 作業完了後、既存の作業レポート（20260720_1558_RES_Phase23-2_FileManager_and_TagManager.md）の「タスク2」の項目に、作業の詳細を追記すること。
5. チャットにて「タスク2の実装が完了しました。ビルド・動作確認をお願いします」と報告すること。

###### 【実装要件】
*  `src/FileManager.cpp` を新規作成し、タスク1およびHotfixで定義した `ExtractTextMetadata` と `ExtractAlbumArtBinary` の2つの静的メソッドの実装を行うこと。
*  旧 `TagManager` で行っていた TagLib を用いたメタデータ（曲名、アーティスト名、曲の長さ）の抽出ロジックを `ExtractTextMetadata` へ、APIC フレーム（アルバムアートのバイナリデータ）の抽出ロジックを `ExtractAlbumArtBinary` へそれぞれ移植し、完全にカプセル化すること。
*  `<taglib/mpegfile.h>`, `<taglib/id3v2tag.h>`, `<taglib/attachedpictureframe.h>` 等の TagLib 関連のインクルードは、必ずこの `.cpp` ファイル内でのみ行うこと。
*  メソッドが「テキスト」と「画像」で分割された利点を活かし、それぞれ呼ばれた際に **必要な情報だけ** を TagLib から読み取る効率的でストイックな処理とすること。

#### 【絶対遵守ルール (Constraints)】
*   **スコープの厳守** : 本タスクの対象は `src/FileManager.cpp` の新規作成および実装のみである。`Application` 等の外部クラスの配線付け替え（タスク3）や、旧 `TagManager` ファイルの削除・CMakeの更新（タスク4）などは次以降のタスクで行うため、絶対に実行しないこと。

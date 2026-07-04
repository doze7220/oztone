【目的】
Phase 9-1: Renderer::Render() の大掃除（関数分割）。
現在の1200行を超える Render() メソッドの中身を、機能（描画要素）ごとにプライベートメソッドとして切り出し、可読性とメンテナンス性を劇的に向上させます。
※このステップでは「関数の抽出」のみを行い、ロジックそのものの変更やクラスの分割は行いません。タスクごとにビルドが通る状態を維持します。

【作業手順（厳守事項）】
1. 以下の【実装要件】を満たす実装計画書を作業レポート（_docs/logs/YYMMDD_HHMM_Phase9-1_Render_ExtractMethods.md）として新規作成する。
2. 作業計画をタスクリストに分割し、作業レポート末尾に記載する。
2. タスクリストの作成が終わったら、絶対にコードの修正は行わずに、まずはチャットで「タスクリストの作成が完了しました。タスク1を実行してよいか指示をお願いします」と報告し、ユーザーの承認を待つこと。

【実装要件】
src/Renderer.h および src/Renderer.cpp を対象とし、以下のプライベートメソッドを定義して、現在の Render() 内の該当処理を移動させてください。Render() メソッド内からはこれらのメソッドを順に呼び出すだけのクリーンな構造にします。

引数について: 各メソッドには、現在の Render() に渡されている引数（isHovered, progress, volume 等）のうち、その描画処理に必要なものだけを適切に渡す設計にすること。

以下の4つのタスクに分けて進行します。
- タスク1: 背景・ビジュアル系の抽出
  - DrawBackground()
  - DrawVisualizer()
- タスク2: 基本UI系の抽出
  - DrawAppLogo()
  - DrawTrackInfo()
  - DrawNextTrack()
- タスク3: 下部コントロール系の抽出
  - DrawSeekBar()
  - DrawPlaybackControls()
  - DrawVolumeControl()
- タスク4: 特殊UI系の抽出
  - DrawPlaylist()
  - DrawResizeGrip()

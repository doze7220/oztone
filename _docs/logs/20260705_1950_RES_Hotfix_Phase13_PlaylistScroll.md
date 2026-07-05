# RES:実装計画・作業レポート Hotfix_Phase13: プレイリストのスクロール計算一元化

## 1. 実装目的
Phase 13（プレイリスト階層化とツールバー追加）で混入した、プレイリストの「クリック判定」と「描画」におけるスクロール計算の二重管理を解消し、ウィンドウ高さが低い場合にクリック判定が大きくズレる不具合をアーキテクチャレベルで根本解決する。

**【重要】**
本件は `20260704_2314_RES_Phase13_PlaylistHierarchy.md` で混入したスクロール計算の二重管理バグに対するHotfixである。

## 2. アーキテクチャ設計
### 要件1: 判定ロジックの一元化
- `LayoutCalculator` に、論理Y座標からクリックされたプレイリストのインデックスを数学的に逆算するメソッド `GetPlaylistItemIndexAt` を追加し、`CalculatePlaylistLayout` 内の計算と完全に整合させる。
- `Application.cpp` 側の `SetPlaylistClickCallback` / `SetPlaylistDoubleClickCallback` で個別に行っていたインデックス計算を廃止し、`LayoutCalculator::GetPlaylistItemIndexAt` へ委譲する。

## 3. 実装タスクリスト
- [x] タスク1: `LayoutCalculator` へのインデックス逆算メソッド新設
- [x] タスク2: `Application.cpp` の判定ロジックの浄化

## 4. 詳細作業内容
### タスク1: `LayoutCalculator` へのインデックス逆算メソッド新設
- `LayoutCalculator.h` および `LayoutCalculator.cpp` に `GetPlaylistItemIndexAt` を追加した。
- ツールバーの高さを考慮し、`PlaylistLayout` 構造体が持つ `startY` および `itemHeight` を利用してY座標からインデックスを逆算する処理を実装した。

### タスク2: `Application.cpp` の判定ロジックの浄化
- `Application.cpp` に `LayoutCalculator.h` をインクルードした。
- `SetPlaylistClickCallback` および `SetPlaylistDoubleClickCallback` 内のクリック判定処理を改修した。
- クライアント領域の論理サイズを正確に取得し、`LayoutCalculator::CalculatePlaylistLayout` でレイアウト情報を取得した上で `LayoutCalculator::GetPlaylistItemIndexAt` を呼び出すように変更し、スクロール計算ロジックの一元化を達成した。

## 5. HOTFIX1
### 原因・理由: 20260704_2314_RES_Phase13_PlaylistHierarchy.md での実装漏れ・バグ
- `20260704_2314_RES_Phase13_PlaylistHierarchy.md` による階層化実装時に、`Application.cpp` 側と `LayoutCalculator` 側で別々のスクロール計算ロジック（二重管理）が残存してしまった。
- その結果、ウィンドウ高さが低い場合等のスクロールクランプ計算結果にズレが生じ、クリック判定が実際の描画位置と合わなくなっていた。

### 対応: ロジックの一元化
- インデックス逆算ロジックを `LayoutCalculator` に新設・完全委譲し、実際の描画に用いられる `PlaylistLayout` と完全に同一の情報を元にインデックス逆算を行うようにアーキテクチャを修正した。

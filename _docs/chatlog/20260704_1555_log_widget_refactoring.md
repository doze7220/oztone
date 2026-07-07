結論から言うと、**1500行だから分割する、という理由ではまだ早い**かな。

今の君のプレイヤーは、以前の「Rendererに全部載っていた」状態からWidget化まで進めたわけだよね。
その段階では**「機能単位でまとまっているか」**の方が、ファイルサイズより重要だと思う。

この `Widgets.cpp` を見る限り、

* AppLogoWidget
* LogoMenuWidget
* TrackInfoWidget
* NextTrackWidget
* SeekBarWidget
* PlaybackControlsWidget
* VolumeControlWidget
* PlaylistWidget

と、クラス単位で綺麗に並んでいる。

つまり、

> Widgets.cpp = 「Widget実装集」

という役割は崩れていない。

---

## ただし PlaylistWidget だけは別

逆に気になったのはここ。

PlaylistWidgetは

* リソース生成
* アニメーション
* スクロール管理
* プレイリスト一覧表示
* 曲一覧表示
* ツールバー
* ハイライト
* ListViewモード
* 通常モード

全部持ってる。

これはもう

> PlaylistWidget が巨大

なのであって、

> Widgets.cpp が巨大

ではない。

---

## もし分割するなら

俺なら

```
Widgets.cpp
```

ではなく

```
Widgets/
    AppLogoWidget.cpp
    LogoMenuWidget.cpp
    TrackInfoWidget.cpp
    NextTrackWidget.cpp
    SeekBarWidget.cpp
    PlaybackControlsWidget.cpp
    VolumeControlWidget.cpp
    PlaylistWidget.cpp
```

こうする。

これは

> 「Widget一つ＝1cpp」

になるので非常に分かりやすい。

---

## さらにPlaylistだけ見ると

PlaylistWidgetだけはさらに

```
PlaylistWidget.cpp
PlaylistWidgetToolbar.cpp
PlaylistWidgetDraw.cpp
PlaylistWidgetLayout.cpp
PlaylistWidgetListView.cpp
```

くらいまで割れる。

ただしこれは

**まだやらなくてもいい**。

---

## 判断基準

俺がいつも使う基準はこれ。

✅ 2000行だから分割
→ ×

✅ 編集すると毎回500行スクロールする
→ ○

✅ Git差分を見るのが辛い
→ ○

✅ 同じファイルをAIが毎回全部読む
→ ◎

---

## AI開発という観点では

ここが一番重要。

例えばPlaylistWidgetだけ修正したいのに

```
Widgets.cpp
1500行
```

を毎回AIが読む。

これは

* トークン増える
* 理解コスト増える
* コンテキスト圧迫

になる。

逆に

```
PlaylistWidget.cpp
500行
```

だけ渡せば済む。

AIとの相性はかなり良くなる。

---

なので、今の設計なら私なら次の優先順位にするかな。

1. **今は無理に分割しない。**
2. **PlaylistWidgetだけがさらに肥大化したら、まずPlaylistWidgetを別ファイル化する。**
3. **最終的には「Widget1クラス＝Widget1.cpp」にする。**

その構成なら、機能ごとの独立性も高く、AIに作業を依頼するときも対象を最小限に絞れるので、今の開発スタイルとも相性が良いと思うよ。

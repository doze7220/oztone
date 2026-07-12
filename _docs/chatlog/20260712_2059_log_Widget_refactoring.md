AI-IDEで開発していて、コンテキストのハルシネーションを防ぎ、かつクォータの節約をするためにも、こいつをリファクタリングしようと思ってる。 基本的には整理→ファイル分割、だね。 どういう方針で進めたらいいだろうか。

AI-IDEを前提にするなら、**「人間が読みやすい」より「AIが1ファイルだけ読めば理解できる」**を優先したほうがいい。

以前の `Application.cpp` や `Window.cpp` の分割でも話したけど、AI-IDEで一番クォータを消費するのは

* 巨大ファイルを毎回読むこと
* 関係ないコードまでコンテキストに入ること
* 修正のたびに2000～3000行を送り直すこと

だから、**変更単位＝ファイル単位**に近づけるのが理想。

---

## このWidgets.cppを見る限り

今の構造は

```
Widgets.cpp
    AppLogoWidget
    TrackInfoWidget
    NextTrackWidget
    SeekBarWidget
    ...
    GlobalHotkeysWidget
    ResizeGripWidget
```

という「全部入り」になっている。

Widget同士はほぼ独立している。

つまり、

> **分割しない理由がほぼない。**

---

## 第一段階

まず共通コードだけ取り出す。

例えば

```
namespace {

ActionID

LoadBitmapResourceHelper()

}
```

これはWidgetじゃない。

なので

```
WidgetCommon.cpp
WidgetCommon.h
```

へ。

```
WidgetCommon.h

bool LoadBitmapResourceHelper(...);

enum ActionID
{
...
};
```

---

## 第二段階

Widgetを完全に1ファイル1Widgetにする。

例えば

```
Widgets/

AppLogoWidget.cpp
TrackInfoWidget.cpp
NextTrackWidget.cpp
SeekBarWidget.cpp
PlaybackControlsWidget.cpp
VolumeControlWidget.cpp
PlaylistWidget.cpp
GlobalHotkeysWidget.cpp
ResizeGripWidget.cpp
```

これだけでもかなり違う。

AIは

> 「TrackInfoだけ修正」

なら

```
TrackInfoWidget.cpp
TrackInfoWidget.h
```

しか読まなくなる。

---

## 第三段階（おすすめ）

実はさらに分けられるWidgetがある。

例えば

```
TrackInfoWidget
```

は

```
CreateResources()

ReleaseResources()

UpdateLayout()

Draw()
```

の4責務ある。

でもここまでは分けなくていい。

AI的にも

```
TrackInfoWidget.cpp
```

が400行くらいなら十分扱える。

---

## 分けるべきなのは巨大Widget

特に以前見せてもらった

```
PlaylistWidget
```

あれ。

あれは1500行近かった。

あれだけは

```
PlaylistWidget_Draw.cpp
PlaylistWidget_Input.cpp
PlaylistWidget_Layout.cpp
PlaylistWidget_Model.cpp
```

くらいまで分けてもいい。

理由は、

Playlistだけは

* 描画
* マウス入力
* スクロール
* D&D
* Playlist切替
* ListView

全部持っているから。

---

## さらにAI向けにするなら

例えばTrackInfo。

今は

```
CreateResources()

Draw()
```

の中に

```
TextLayout生成
```

が混ざっている。

例えば

```
TrackInfoText.cpp

CreateTitleLayout()

CreateArtistLayout()

CreateTrackCountLayout()
```

へ逃がすこともできる。

でもこれはやり過ぎ。

---

## 逆に分けないもの

例えば

```
ResizeGripWidget
```

100行。

これはそのままでいい。

AIも困らない。

---

## AI-IDE最適化としての優先順位

私ならこの順でやる。

```
① WidgetCommon.cppを作る
    ↓
② 1Widget = 1cpp
    ↓
③ Playlistだけ責務分割
    ↓
④ 必要になったWidgetだけ追加分割
```

これが一番コスパがいい。

---

## 今後を考えるならディレクトリ構成も少し変える

将来的にWidgetが増えるなら、ファイル名だけでなくカテゴリごとに分けるとAIにも分かりやすい。

```
Widgets/
├── Common/
│   ├── WidgetCommon.cpp
│   └── WidgetCommon.h
├── Header/
│   ├── AppLogoWidget.cpp
│   ├── TrackInfoWidget.cpp
│   └── NextTrackWidget.cpp
├── Controls/
│   ├── SeekBarWidget.cpp
│   ├── PlaybackControlsWidget.cpp
│   └── VolumeControlWidget.cpp
├── Playlist/
│   ├── PlaylistWidget.cpp
│   ├── PlaylistInput.cpp
│   ├── PlaylistDraw.cpp
│   └── PlaylistLayout.cpp
└── Utility/
    ├── GlobalHotkeysWidget.cpp
    └── ResizeGripWidget.cpp
```

この構成だと、AIは「プレイリスト関連だけ」「コントロール関連だけ」といった単位でコンテキストを絞りやすくなります。

---

### このプロジェクト向けのおすすめ方針

あなたの目的は**可読性のためのリファクタリングではなく、AI-IDEのコンテキスト節約**なので、**「変更が発生する責務ごとに物理ファイルを分ける」**ことを基準にするのが適しています。

その観点では、

* `WidgetCommon.cpp` を新設して共通処理を集約する。
* **各 Widget は 1 Widget = 1 .cpp** に分割する。
* 500～700行を超える Widget（特に `PlaylistWidget`）だけは、描画・入力・レイアウトなどの責務でさらに分割する。
* 100～200行程度の小さな Widget は無理に細分化しない。

このバランスが、クォータ消費・ハルシネーション抑制・保守性の3つを最も両立できると思う。

ファイル分割するのはコンテキスト節約だけではない。
スコープを絞り、作業に必要のないテキストを読ませないことにより、作業をより単純化・スコープの焦点化も兼ねている。
一挙両得、ってやつだな。

その考え方には賛成だ。

実はAI-IDEでのファイル分割って、従来の「責務分離」だけじゃなく、もう一つ重要な目的がある。

**「AIの思考空間（コンテキスト）を制限すること」**だ。

例えば今の `Widgets.cpp` を考えると、

```
AppLogoWidget
TrackInfoWidget
NextTrackWidget
SeekBarWidget
PlaybackControlsWidget
VolumeControlWidget
PlaylistWidget
GlobalHotkeysWidget
ResizeGripWidget
```

これだけが1ファイルにある。

仮に

> 「SeekBarのドラッグ処理だけ修正して」

というタスクでも、AIは

* AppLogo
* TrackInfo
* NextTrack
* Playlist
* Hotkeys

まで全部読む可能性がある。

人間なら「関係ない」と切り捨てられるが、LLMは**コンテキストに存在する情報を無意識に参照してしまう**。

これが、

* 関係ないコードへ修正が飛ぶ
* 命名規則が混ざる
* 修正範囲が広がる
* ハルシネーションが増える

原因になる。

---

なので、AI向けには

```
SeekBarWidget.cpp
```

しか渡さなければ、

AIの頭の中には

```
SeekBarしか存在しない
```

状態になる。

これは単なるトークン節約以上に、

**「問題空間そのものを小さくする」**

効果がある。

---

だから、AI-IDE向けの設計指針は従来のSOLIDとは少し違う。

私はこんな優先順位で考えている。

```
① AIが1タスクで読む量を減らす

② 変更範囲を物理的に閉じ込める

③ コンテキストの混線を防ぐ

④ 人間の保守性を上げる

⑤ トークン節約
```

普通の開発では⑤は存在しない。

AI開発では⑤も重要だけど、実際には①〜③の恩恵のほうが大きい。

---

だから最近あなたが進めている

* Renderer分割
* Window分割
* Application分割
* Widget分割

は、一見すると全部「ファイル分割」に見えるけれど、本質は

> **AIがそのタスクだけを理解できる「作業単位」を作っている**

ことなんだ。

これは従来の「責務分離」と似ているようで少し違う。

むしろ私はこれを**「コンテキスト境界（Context Boundary）」を設計する**作業だと考えている。

つまり、「クラスの責務」ではなく、「AIが一度に考える責務」を設計しているわけだ。

この視点で見ると、「1 Widget = 1 ファイル」は単なる整理ではなく、**AIに対するインターフェース設計**になる。今後さらにAI-IDEを前提に開発を続けるなら、この考え方は長期的にもかなり相性がいいと思う。

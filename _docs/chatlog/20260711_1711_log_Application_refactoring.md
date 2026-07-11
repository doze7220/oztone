Apprication.cppリファクタリング相談。 以下のファイルをリファクタリングすべきだろうか？ またするとしたらどういう方針がいいだろうか？

結論から言うと、**これはリファクタリングすべき**だね。
ただし、**新しいクラスを大量に作る段階ではなく、「重複除去」と「責務整理」だけで十分**だと思う。

今のコードは設計そのものは悪くない。

* Windowは入力を受け取るだけ
* Rendererは描画だけ
* PlaylistManagerはプレイリスト管理
* AudioPlayerは再生

という責務はかなり綺麗に分かれている。

一方で **Applicationだけが何でも知りすぎている**。

---

## 一番大きな問題

まず一番気になったのはこれ。

例えば

```
HandleMediaCommand()
```

```
SwitchPlaylist()
```

```
OnFilesDropped()
```

```
Run()
```

```
PlaylistClick()
```

全部に

```
タグ読む

↓

Rendererへ反映

↓

背景設定

↓

Play()

↓

UpdateTrackMetadataIfNeeded()

↓

PrefetchNextTrack()
```

この処理がそのままコピーされている。

---

ざっと見るだけでも

* HandleMediaCommand
* PlaylistClick
* RemoveCurrentTrack
* Initialize
* OnFilesDropped
* Run(曲終了)
* SwitchPlaylist

少なくとも7か所ある。

これはかなり危険。

例えば

```
背景の設定方法変えよう
```

となると

7か所直す必要がある。

---

## 最優先

例えば

```
bool Application::PlayCurrentTrack(bool usePrefetch)
```

みたいなのを作る。

中身は

```
現在曲取得

↓

タグ取得

↓

Renderer反映

↓

背景

↓

Play()

↓

Metadata更新

↓

Prefetch
```

全部ここ。

すると

今ある

```
while(skipCount < totalCount)
{
    ...

    if (PlayCurrentTrack(...))
        break;

    Advance();
}
```

くらいまで縮む。

これはかなり効く。

---

## 次に

タグ表示も完全に重複している。

何回出てきたか分からない。

```
if (m_tagManager.Load(track))
{
    ...
}
else
{
    ...
}
```

これだけで300行近くある。

例えば

```
UpdateTrackDisplay(track)
```

だけでいい。

```
UpdateTrackDisplay(track);
```

---

## 背景も同じ

毎回

```
float ox;
float oy;
float scale;

GetArtFraming()

SetBackgroundFraming()
```

これも

```
UpdateBackgroundFraming(track);
```

一行で済む。

---

## Initialize()

ここもかなり長い。

実際に読んでみると

ほぼ

```
SetXXXCallback()
```

しかしていない。

なので

例えば

```
RegisterWindowCallbacks();
```

に全部移せる。

Initialize()は

```
LoadDatabase()

InitializeConfig()

InitializeWindow()

RegisterCallbacks()

InitializeRenderer()

InitializeAudio()

InitializeTrackAnalyzer()

UpdatePlaylistSummaries()

return true;
```

くらいになる。

見通しがかなり良くなる。

---

## ForceRender()

これも責務が多い。

今やっていることは

* 時間表示
* Progress計算
* Spectrum取得
* Hover判定
* LogoMenu処理
* Animation更新
* Metadata生成
* Render

全部ある。

例えば

```
UpdatePlaybackInfo();
```

```
UpdateHoverState();
```

```
HandleLogoMenuEvents();
```

```
BuildMetadataList();
```

くらいに分けるだけでも十分。

---

## OnFilesDropped()

ここは

```
ファイル探索

↓

Playlist追加

↓

Analyzer追加

↓

再生開始
```

4つの責務がある。

さらに

```
IsSupportedAudioFile()

IsValidAudioFile()
```

まで中にある。

これは

```
Application.cpp
```

ではなく

```
FileDropHelper.cpp
```

くらいに逃がしてもいい。

---

## ラムダが巨大

例えば

```
SetPlaylistClickCallback
```

200行近い。

これは

```
m_window.SetPlaylistClickCallback(
    [this](int x,int y)
    {
        OnPlaylistClicked(x,y);
    });
```

だけで十分。

中身は

```
Application::OnPlaylistClicked()
```

へ。

---

## 個人的に一番やりたい順番

優先順位を付けるなら

① 再生処理共通化 ★★★★★

```
PlayCurrentTrack()

UpdateTrackDisplay()

UpdateBackground()
```

---

② Initialize分割 ★★★★★

```
RegisterCallbacks()

InitializeAudio()

InitializeRenderer()
```

---

③ ForceRender分割 ★★★★

```
UpdateHover()

BuildMetadata()

HandleLogoMenu()
```

---

④ Playlistクリック分離 ★★★★

```
OnPlaylistClick()

OnPlaylistDoubleClick()

OnToolbarClick()
```

---

⑤ FileDrop整理 ★★★

---

## やらない方がいいこと

今の段階では

```
PlaybackController

PlaylistController

WindowController

InputController

ApplicationService

CommandDispatcher
```

みたいなクラスを増やすのはおすすめしない。

今のコードはApplicationが司令塔になっているので、

> **「Applicationは残したまま、Applicationの中身を小さなメンバ関数へ分割する」**

これが一番コストに対して効果が高い。

---

## 全体評価

このファイルは設計自体よりも**重複コードの多さ**が保守性を下げている印象だ。

おすすめのリファクタリング順は次のとおり。

1. **再生処理を `PlayCurrentTrack()` に集約**（効果最大）
2. **タグ・アルバムアート表示を `UpdateTrackDisplay()` に集約**
3. **背景設定を `UpdateBackgroundFraming()` に集約**
4. **`Initialize()` を「初期化」と「コールバック登録」に分割**
5. **巨大なコールバックラムダを専用メンバ関数へ移動**
6. **`ForceRender()` を描画前準備ごとに小さな関数へ分割**

この方針ならアーキテクチャを崩さず、ファイルサイズと重複を大きく減らせるはずだ。

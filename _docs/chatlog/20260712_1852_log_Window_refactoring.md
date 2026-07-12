AI-IDEで開発していて、コンテキストのハルシネーションを防ぎ、かつクォータの節約をするためにも、こいつをリファクタリングしようと思ってる。 基本的には整理→ファイル分割、だね。 どういう方針で進めたらいいだろうか。

AI-IDE前提なら、**「責務」よりも「コンテキスト単位」で分割する**のが一番効果的だと思う。

この `Window.cpp` を見る限り、今は **Windowクラスが「Windows APIとの窓口」を全部抱えている**状態になっている。
これは人間には読めても、AIにはかなり厳しい。

AIは

* WM_MOUSEMOVEだけ見たい
* トレイメニューだけ見たい
* ホットキーだけ見たい

という時でも、2000行近いファイル全体を読むことになる。

そのため、**「この機能を変更したいなら、このcppだけ渡せば十分」**という状態を目指すのがいい。

---

## まず最初に分けるべきもの

### ① WindowProc系（最優先）

今一番巨大なのは

```cpp
LRESULT Window::WindowProc(...)
```

これ。

ここだけで数百行ある。

これは

```
Window_Mouse.cpp
Window_Command.cpp
Window_Tray.cpp
Window_Message.cpp
```

みたいに分割したい。

例えば

```
Window_Mouse.cpp

HandleMouseMove()
HandleLButtonDown()
HandleMouseWheel()
HandleMouseLeave()
```

---

```
Window_Tray.cpp

BuildTrayMenu()
HandleTrayCommand()
```

---

```
Window_Command.cpp

HandleCommand()
HandleHotkey()
HandleCopyData()
```

---

```
Window_System.cpp

HandleDestroy()
HandleSize()
HandleMinMax()
```

みたいな感じ。

すると

```
WM_MOUSEMOVEを修正して
```

と言われたAIは

```
Window_Mouse.cpp
```

しか読まなくなる。

これはかなりトークン削減になる。

---

## ② HitTest群

これもかなりまとまってる。

```
IsInLogoRegion()

IsInLogoMenuRegion()

IsInPlaybackControlRegion()

IsInVolumeControlRegion()

IsInPlaylistRegion()

GetPlaybackButtonAt()

GetPlaylistToolbarButtonAt()
```

全部

```
WindowHitTest.cpp
```

にしていい。

共通しているのは

> 「座標からUI部品を判定する」

だけだから。

---

## ③ Tray Menu

ここも独立できる。

今は

```
WM_TRAYICON
```

の中に

```
CreatePopupMenu

AppendMenu

Playlist生成

Advanced生成

Volume生成
```

全部ある。

これは

```
TrayMenuBuilder.cpp
```

でもいいくらい。

例えば

```
HMENU BuildTrayMenu();
```

だけになれば、

Window.cppはかなり細くなる。

---

## ④ DropTarget

これもWindow.cppにある必要がない。

```
DropTarget.cpp
DropTarget.h
```

へ。

完全に別責務。

---

## ⑤ Initialize

これも結構長い。

中身を見ると

```
Register WindowClass

↓

Window生成

↓

HotKey登録

↓

DragDrop

↓

TrayIcon

```

なので

```
InitializeWindow()

InitializeTray()

InitializeDropTarget()

InitializeHotkeys()
```

くらいに分けられる。

---

# 逆に分けない方がいいもの

例えば

```
RegisterHotkeys()

UnregisterHotkeys()
```

これは短い。

分けても

```
Window_Hotkey.cpp
```

が50行になるだけ。

AI的にもメリットが薄い。

---

# AI向けならもっと重要なのは

実は

**「依存方向」**

なんだ。

例えば

```
Window_Mouse.cpp
```

が

```
Renderer

AudioPlayer

Playlist

Visualizer

```

全部includeしてたら意味がない。

理想は

```
Window_Mouse.cpp

↓

Window.h

↓

std::function callback
```

だけ。

つまり

```
Windowはイベントを通知するだけ
```

にする。

今も結構そうなってるので、この設計は維持したい。

---

# さらにAI向けにするなら

俺ならここまでやる。

```
Window/

    Window.cpp
        Initialize
        ProcessMessage

    Window_Mouse.cpp
    Window_HitTest.cpp
    Window_Tray.cpp
    Window_Command.cpp
    Window_System.cpp
    Window_Hotkey.cpp
    Window_DragDrop.cpp
```

すると

例えば

```
Shift+ホイールで背景を拡縮する処理を変えたい
```

なら

AIには

```
Window_Mouse.cpp
```

だけ渡せば済む。

---

## このファイルなら分割優先順位はこう付ける

1. **WindowProcのイベント処理**（最も効果が大きい）
2. **HitTest群**（座標判定を完全に独立）
3. **TrayMenu生成・コマンド処理**（メニュー機能を独立）
4. **DropTarget**（完全に別責務なので切り出しやすい）
5. **Initialize関連**（初期化処理の整理）
6. **Hotkey**（最後で十分）

この順番なら、最初の3段階だけでも `Window.cpp` は半分以下のサイズになり、AI-IDEでのコンテキスト消費とハルシネーションの抑制にかなり効いてくると思う。

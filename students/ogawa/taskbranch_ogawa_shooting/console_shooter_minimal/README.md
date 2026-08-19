# Minimal Console Shooter

Windows 10/11 と MinGW GCC 向けの C 言語コンソールシューティングです。

## ビルド

MinGW のコマンドプロンプトまたは PowerShell で実行します。

```text
gcc -std=c11 -Wall -Wextra -O2 -o console_shooter.exe main.c game.c input.c render.c
```

または、`mingw32-make` がある場合は次のコマンドを使えます。

```text
mingw32-make
```

既定の `CreateThread`、`CRITICAL_SECTION`、`QueryPerformanceCounter` を使用しているため、pthread や外部ライブラリの追加リンクは不要です。

## 操作

- `A` / `D` または左右矢印: 移動
- `SPACE`: ミサイル発射
- `Q` / `ESC`: 終了

ゲーム設定は `game.h` 冒頭の `#define` にまとめています。ANSI/VT 対応コンソールでは、毎フレームに画面バッファ全体を再構築して `ESC[H` から出力します。オブジェクトの移動は ANSI ではなく、ゲームスレッドが座標を変更して実現しています。
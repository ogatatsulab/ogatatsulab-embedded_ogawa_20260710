# UNO R3 タクトスイッチのチャタリング対策

## 配線

- タクトスイッチの片側を UNO R3 の `D2`、反対側を `GND` に接続します。
- `INPUT_PULLUP` を使うため、未押下は `HIGH`、押下は `LOW` です。
- 内蔵 LED は `LED_BUILTIN` を使います。

## Arduino IDE

`tact_switch_debounce.ino` を UNO R3 に書き込むと、ボタンを押している間だけ内蔵 LED が点灯します。入力が 30 ms 同じ状態で続いた場合だけ状態を確定するため、押下・解放時のチャタリングを無視します。

## ホスト側テスト

このフォルダーで次を実行します。

```text
gcc -std=c11 -Wall -Wextra -pedantic debounce_test.c -o debounce_test.exe
debounce_test.exe
```

テストは、押下時・解放時のチャタリング、`millis()` のオーバーフロー、クリック、ダブルクリック、長押しを確認します。

`updateButton()` はイベントをビットフラグで返します。短いクリックは、次のクリックを待つため `DOUBLE_CLICK_MS` 経過後に確定します。長押しイベントは押下中に一度だけ発生し、長押し後の解放はクリックとして扱いません。

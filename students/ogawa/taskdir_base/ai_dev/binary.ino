// 74HC595 を使う前提のシフトレジスタ制御
// 8個のLEDを 1バイトの2進数として扱う
// 例: 5 -> 00000101

const int latchPin = 8;   // ST_CP (RCLK)
const int clockPin = 10;  // SH_CP (SRCLK)
const int dataPin  = 11;  // DS (SER)

void setup() {
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  Serial.begin(9600);
  Serial.println("LED Binary Counter Start");
}

void loop() {
  for (int value = 0; value <= 255; value++) {
    // ラッチをLOWにしてシフトレジスタへ書き込む準備
    digitalWrite(latchPin, LOW);

    // 8ビットを一括で出力
    // MSBFIRST にすると bit7 から順に送る
    shiftOut(dataPin, clockPin, MSBFIRST, (byte)value);

    // 書き込み完了を知らせる
    digitalWrite(latchPin, HIGH);

    // 10進数の現在値をシリアル表示
    // 2進数は常に8桁表示（例: 00000005）
    // String binaryStr = String(value, BIN);
    // while (binaryStr.length() < 8) {
    //   binaryStr = "0" + binaryStr;
    // }

    Serial.print("Value = ");
    Serial.print(value);
    Serial.print("    Binary = ");
    // Serial.println(binaryStr);
    for (int bit = 7; bit >= 0; bit--) {
      Serial.print((value >> bit) & 1);
    }
    Serial.println();

    delay(500); // 500ms ごとに更新

  }
  
  Serial.println("--- Restart ---");
  delay(1000);
}

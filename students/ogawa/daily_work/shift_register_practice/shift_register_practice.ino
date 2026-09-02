#include <Arduino.h>

const uint8_t DATA_PIN = 2;
const uint8_t CLOCK_PIN = 3;
const uint8_t LATCH_PIN = 4;
const uint8_t BUTTON_PIN = 5;

const uint8_t DEBOUNCE_MS = 25;

uint8_t displayValue = 0b00000000;
uint8_t exerciseIndex = 0;
bool lastButtonState = HIGH;
bool stableButtonState = HIGH;
unsigned long lastChangeAt = 0;

void writeShiftRegister(uint8_t value) {
  digitalWrite(LATCH_PIN, LOW);
  shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, value);
  digitalWrite(LATCH_PIN, HIGH);
}

void printBinary(uint8_t value) {
  for (int i = 7; i >= 0; --i) {
    Serial.print((value >> i) & 1U ? '1' : '0');
  }
  Serial.println();
}

void showValue() {
  writeShiftRegister(displayValue);
  Serial.print(F("value = "));
  Serial.print(displayValue);
  Serial.print(F("  binary = "));
  printBinary(displayValue);
}

void runExercise() {
  Serial.println();
  Serial.println(F("--- Bit Exercise ---"));

  switch (exerciseIndex) {
    case 0:
      displayValue = 0b00000000;
      displayValue |= (1U << 0);
      Serial.println(F("ó˚èK1: bit 0 Ç 1 Ç…Ç∑ÇÈ"));
      Serial.println(F("ó·: value |= (1 << 0);"));
      break;

    case 1:
      displayValue = 0b00001111;
      displayValue &= ~(1U << 3);
      Serial.println(F("ó˚èK2: bit 3 Ç 0 Ç…Ç∑ÇÈ"));
      Serial.println(F("ó·: value &= ~(1 << 3);"));
      break;

    case 2:
      displayValue = 0b10100101;
      displayValue ^= (1U << 6);
      Serial.println(F("ó˚èK3: bit 6 ÇîΩì]Ç∑ÇÈ"));
      Serial.println(F("ó·: value ^= (1 << 6);"));
      break;

    case 3:
      displayValue = 0b00001010;
      displayValue <<= 1;
      Serial.println(F("ó˚èK4: ç∂Ç÷ 1 bit ÉVÉtÉg"));
      Serial.println(F("ó·: value <<= 1;"));
      break;

    case 4:
      displayValue = 0b11010100;
      Serial.print(F("ó˚èK5: bit 2 ÇÃíl = "));
      Serial.println(((displayValue >> 2) & 1U) ? "1" : "0");
      Serial.println(F("ó·: (value >> 2) & 1U"));
      break;

    default:
      exerciseIndex = 0;
      displayValue = 0b00000000;
      Serial.println(F("ç≈èâÇÃó˚èKÇ…ñﬂÇËÇ‹Ç∑ÅB"));
      break;
  }

  showValue();

  if (exerciseIndex < 4) {
    exerciseIndex += 1;
  } else {
    exerciseIndex = 0;
  }
}

void setup() {
  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  digitalWrite(LATCH_PIN, LOW);
  digitalWrite(CLOCK_PIN, LOW);
  digitalWrite(DATA_PIN, LOW);

  Serial.begin(9600);
  while (!Serial) {
    ;
  }

  Serial.println(F("74HC595 Bit Learning"));
  Serial.println(F("É{É^ÉìÇâüÇ∑Ç∆éüÇÃó˚èKÇ…êiÇ›Ç‹Ç∑ÅB"));
  displayValue = 0b00000000;
  writeShiftRegister(displayValue);
  showValue();
}

void loop() {
  const bool reading = digitalRead(BUTTON_PIN) == LOW;
  const unsigned long now = millis();

  if (reading != lastButtonState) {
    lastChangeAt = now;
    lastButtonState = reading;
  }

  if ((now - lastChangeAt) >= DEBOUNCE_MS && reading != stableButtonState) {
    stableButtonState = reading;

    if (stableButtonState == LOW) {
      runExercise();
    }
  }
}

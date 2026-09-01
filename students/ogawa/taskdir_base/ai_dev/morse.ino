#include <Arduino.h>

const uint8_t BUTTON_PIN = 2;
const uint8_t BUZZER_PIN = 8;
const uint8_t LED_PIN = LED_BUILTIN;

const unsigned long DOT_DASH_LIMIT_MS = 300;
const unsigned long CHARACTER_GAP_MS = 1000;
const unsigned long WORD_GAP_MS = 2500;
const unsigned long RESET_HOLD_MS = 3000;
const unsigned long DEBOUNCE_MS = 25;

struct MorseEntry {
  const char *code;
  char letter;
};

const MorseEntry MORSE_TABLE[] = {
  {".-", 'A'}, {"-...", 'B'}, {"-.-.", 'C'}, {"-..", 'D'},
  {".", 'E'}, {"..-.", 'F'}, {"--.", 'G'}, {"....", 'H'},
  {"..", 'I'}, {".---", 'J'}, {"-.-", 'K'}, {".-..", 'L'},
  {"--", 'M'}, {"-.", 'N'}, {"---", 'O'}, {".--.", 'P'},
  {"--.-", 'Q'}, {".-.", 'R'}, {"...", 'S'}, {"-", 'T'},
  {"..-", 'U'}, {"...-", 'V'}, {".--", 'W'}, {"-..-", 'X'},
  {"-.--", 'Y'}, {"--..", 'Z'},
  {"-----", '0'}, {".----", '1'}, {"..---", '2'}, {"...--", '3'},
  {"....-", '4'}, {".....", '5'}, {"-....", '6'}, {"--...", '7'},
  {"---..", '8'}, {"----.", '9'}
};

char currentCode[8] = "";
uint8_t currentCodeLength = 0;
bool stablePressed = false;
bool lastReading = false;
unsigned long lastChangeAt = 0;
unsigned long pressedAt = 0;
unsigned long releasedAt = 0;

void clearCurrentCode() {
  currentCodeLength = 0;
  currentCode[0] = '\0';
}

void printDecodedCharacter() {
  if (currentCodeLength == 0) {
    return;
  }

  for (const MorseEntry &entry : MORSE_TABLE) {
    if (strcmp(currentCode, entry.code) == 0) {
      Serial.print(F(" => "));
      Serial.print(entry.letter);
      clearCurrentCode();
      return;
    }
  }

  Serial.print(F(" => ?"));
  clearCurrentCode();
}

void addSignal(char signal) {
  if (currentCodeLength >= sizeof(currentCode) - 1) {
    return;
  }

  currentCode[currentCodeLength++] = signal;
  currentCode[currentCodeLength] = '\0';
  Serial.print(signal);
}

void resetMessage() {
  clearCurrentCode();
  Serial.println();
  Serial.println(F("[RESET]"));
  releasedAt = millis();
}

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(9600);
  Serial.println(F("Morse transmitter ready"));
  Serial.println(F("short=dot, long=dash, gap=letter, 3s=reset"));
}

void loop() {
  const bool readingPressed = digitalRead(BUTTON_PIN) == LOW;
  const unsigned long now = millis();

  if (readingPressed != lastReading) {
    lastChangeAt = now;
    lastReading = readingPressed;
  }

  if (now - lastChangeAt >= DEBOUNCE_MS && readingPressed != stablePressed) {
    stablePressed = readingPressed;

    if (stablePressed) {
      pressedAt = now;
      tone(BUZZER_PIN, 880);
      digitalWrite(LED_PIN, HIGH);
    } else {
      noTone(BUZZER_PIN);
      digitalWrite(LED_PIN, LOW);
      const unsigned long pressDuration = now - pressedAt;

      if (pressDuration >= RESET_HOLD_MS) {
        resetMessage();
      } else {
        addSignal(pressDuration < DOT_DASH_LIMIT_MS ? '.' : '-');
        releasedAt = now;
      }
    }
  }

  if (!stablePressed && currentCodeLength > 0 && now - releasedAt >= CHARACTER_GAP_MS) {
    printDecodedCharacter();
    releasedAt = now;
  }

  if (!stablePressed && currentCodeLength == 0 && releasedAt != 0 && now - releasedAt >= WORD_GAP_MS) {
    Serial.print(' ');
    releasedAt = 0;
  }
}

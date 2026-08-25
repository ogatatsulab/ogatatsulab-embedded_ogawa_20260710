#include "debounce.h"

const uint8_t BUTTON_PIN = 2;
const uint8_t LED_PIN = LED_BUILTIN;
const unsigned long DEBOUNCE_MS = 30;
const unsigned long LONG_PRESS_MS = 1000;
const unsigned long DOUBLE_CLICK_MS = 400;

ButtonState button_state;

void setup()
{
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    button_init(&button_state, false, millis());
}

void loop()
{
    unsigned long now_ms = millis();
    ButtonEvent event = updateButton(&button_state,
                                     digitalRead(BUTTON_PIN) == LOW,
                                     now_ms, DEBOUNCE_MS, LONG_PRESS_MS,
                                     DOUBLE_CLICK_MS);
    digitalWrite(LED_PIN, button_state.pressed ? HIGH : LOW);

    if (event & BUTTON_EVENT_LONG_PRESS) {
        // í∑âüÇµéûÇÃèàóùÇÇ±Ç±Ç…í«â¡ÇµÇ‹Ç∑ÅB
    }
}

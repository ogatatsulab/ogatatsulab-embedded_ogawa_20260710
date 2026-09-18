#include <board.h>

void setup() {
}

void loop() {
    while (1) {
        digitalWrite(PIN_LED1, HIGH);
        delay(1000);
        digitalWrite(PIN_LED1, LOW);
        delay(1000);
    }
}

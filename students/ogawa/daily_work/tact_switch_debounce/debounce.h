#ifndef TACT_SWITCH_DEBOUNCE_H
#define TACT_SWITCH_DEBOUNCE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool stable_state;
    bool last_reading;
    unsigned long last_change_ms;
} DebounceState;

typedef enum {
    BUTTON_EVENT_NONE = 0,
    BUTTON_EVENT_CLICK = 1 << 0,
    BUTTON_EVENT_DOUBLE_CLICK = 1 << 1,
    BUTTON_EVENT_LONG_PRESS = 1 << 2
} ButtonEvent;

typedef struct {
    DebounceState debounce;
    bool pressed;
    bool long_press_reported;
    uint8_t click_count;
    unsigned long pressed_at_ms;
    unsigned long last_release_ms;
} ButtonState;

static inline void debounce_init(DebounceState *state, bool initial_state,
                                 unsigned long now_ms)
{
    state->stable_state = initial_state;
    state->last_reading = initial_state;
    state->last_change_ms = now_ms;
}

static inline bool updateDebounce(DebounceState *state, bool reading,
                                  unsigned long now_ms,
                                  unsigned long debounce_ms)
{
    if (reading != state->last_reading) {
        state->last_reading = reading;
        state->last_change_ms = now_ms;
    }

    if ((unsigned long)(now_ms - state->last_change_ms) >= debounce_ms) {
        state->stable_state = state->last_reading;
    }

    return state->stable_state;
}

static inline void button_init(ButtonState *state, bool initial_pressed,
                               unsigned long now_ms)
{
    debounce_init(&state->debounce, initial_pressed, now_ms);
    state->pressed = initial_pressed;
    state->long_press_reported = initial_pressed;
    state->click_count = 0;
    state->pressed_at_ms = now_ms;
    state->last_release_ms = now_ms;
}

static inline ButtonEvent updateButton(ButtonState *state, bool reading,
                                       unsigned long now_ms,
                                       unsigned long debounce_ms,
                                       unsigned long long_press_ms,
                                       unsigned long double_click_ms)
{
    ButtonEvent event = BUTTON_EVENT_NONE;
    bool pressed = updateDebounce(&state->debounce, reading, now_ms,
                                  debounce_ms);

    if (pressed && !state->pressed) {
        state->pressed = true;
        state->pressed_at_ms = now_ms;
        state->long_press_reported = false;
    } else if (!pressed && state->pressed) {
        state->pressed = false;
        state->last_release_ms = now_ms;
        if (!state->long_press_reported) {
            state->click_count++;
            if (state->click_count == 2) {
                event = BUTTON_EVENT_DOUBLE_CLICK;
                state->click_count = 0;
            }
        }
    }

    if (state->pressed && !state->long_press_reported &&
        (unsigned long)(now_ms - state->pressed_at_ms) >= long_press_ms) {
        state->long_press_reported = true;
        state->click_count = 0;
        event = (ButtonEvent)(event | BUTTON_EVENT_LONG_PRESS);
    }

    if (!state->pressed && state->click_count == 1 &&
        (unsigned long)(now_ms - state->last_release_ms) >= double_click_ms) {
        state->click_count = 0;
        event = (ButtonEvent)(event | BUTTON_EVENT_CLICK);
    }

    return event;
}

#ifdef __cplusplus
}
#endif

#endif

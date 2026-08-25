#include <assert.h>
#include <stdio.h>

#include "debounce.h"

static void test_ignores_bounce_before_timeout(void)
{
    DebounceState state;
    debounce_init(&state, false, 0);

    assert(updateDebounce(&state, true, 1, 30) == false);
    assert(updateDebounce(&state, false, 2, 30) == false);
    assert(updateDebounce(&state, true, 3, 30) == false);
    assert(updateDebounce(&state, true, 33, 30) == true);
}

static void test_release_requires_stable_time(void)
{
    DebounceState state;
    debounce_init(&state, true, 100);

    assert(updateDebounce(&state, false, 101, 30) == true);
    assert(updateDebounce(&state, false, 129, 30) == true);
    assert(updateDebounce(&state, false, 131, 30) == false);
}

static void test_millis_wraparound(void)
{
    DebounceState state;
    debounce_init(&state, false, 0xFFFFFFF0UL);

    assert(updateDebounce(&state, true, 0xFFFFFFF5UL, 20) == false);
    assert(updateDebounce(&state, true, 0x00000009UL, 20) == true);
}

static void test_click_and_double_click(void)
{
    ButtonState state;
    button_init(&state, false, 0);

    assert(updateButton(&state, true, 1, 10, 1000, 400) == BUTTON_EVENT_NONE);
    assert(updateButton(&state, true, 11, 10, 1000, 400) == BUTTON_EVENT_NONE);
    assert(updateButton(&state, false, 20, 10, 1000, 400) == BUTTON_EVENT_NONE);
    assert(updateButton(&state, false, 30, 10, 1000, 400) == BUTTON_EVENT_NONE);
    assert(updateButton(&state, true, 100, 10, 1000, 400) == BUTTON_EVENT_NONE);
    assert(updateButton(&state, true, 110, 10, 1000, 400) == BUTTON_EVENT_NONE);
    assert(updateButton(&state, false, 120, 10, 1000, 400) == BUTTON_EVENT_NONE);
    assert(updateButton(&state, false, 130, 10, 1000, 400) == BUTTON_EVENT_DOUBLE_CLICK);
}

static void test_long_press_is_reported_once(void)
{
    ButtonState state;
    button_init(&state, false, 0);

    assert(updateButton(&state, true, 1, 10, 100, 400) == BUTTON_EVENT_NONE);
    assert(updateButton(&state, true, 11, 10, 100, 400) == BUTTON_EVENT_NONE);
    assert(updateButton(&state, true, 111, 10, 100, 400) == BUTTON_EVENT_LONG_PRESS);
    assert(updateButton(&state, true, 500, 10, 100, 400) == BUTTON_EVENT_NONE);
    assert(updateButton(&state, false, 510, 10, 100, 400) == BUTTON_EVENT_NONE);
    assert(updateButton(&state, false, 520, 10, 100, 400) == BUTTON_EVENT_NONE);
}

static void test_bounce_does_not_create_click(void)
{
    ButtonState state;
    button_init(&state, false, 0);

    assert(updateButton(&state, true, 1, 30, 1000, 400) == BUTTON_EVENT_NONE);
    assert(updateButton(&state, false, 2, 30, 1000, 400) == BUTTON_EVENT_NONE);
    assert(updateButton(&state, true, 3, 30, 1000, 400) == BUTTON_EVENT_NONE);
    assert(updateButton(&state, true, 33, 30, 1000, 400) == BUTTON_EVENT_NONE);
    assert(updateButton(&state, false, 34, 30, 1000, 400) == BUTTON_EVENT_NONE);
    assert(updateButton(&state, false, 64, 30, 1000, 400) == BUTTON_EVENT_NONE);
    assert(updateButton(&state, false, 464, 30, 1000, 400) == BUTTON_EVENT_CLICK);
}

int main(void)
{
    test_ignores_bounce_before_timeout();
    test_release_requires_stable_time();
    test_millis_wraparound();
    test_click_and_double_click();
    test_long_press_is_reported_once();
    test_bounce_does_not_create_click();
    puts("All debounce tests passed.");
    return 0;
}

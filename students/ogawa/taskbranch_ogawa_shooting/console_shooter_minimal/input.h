#ifndef INPUT_H
#define INPUT_H

#include <windows.h>
#include "game.h"

typedef struct {
    InputState state;
    CRITICAL_SECTION lock;
    volatile LONG stop_requested;
} InputContext;

void input_init(InputContext *input);
HANDLE input_start(InputContext *input);
InputState input_take(InputContext *input);
void input_stop(InputContext *input);
void input_destroy(InputContext *input);

#endif
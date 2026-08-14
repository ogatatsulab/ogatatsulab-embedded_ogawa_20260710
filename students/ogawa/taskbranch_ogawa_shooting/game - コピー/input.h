#ifndef INPUT_H
#define INPUT_H

#include <stdatomic.h>
#include <stdbool.h>
#include <windows.h>

typedef struct {
    /* atomic変数：入力状態 */
    atomic_bool key_left;      /* A or ← */
    atomic_bool key_right;     /* D or → */
    atomic_bool key_up;        /* W or ↑ */
    atomic_bool key_down;      /* S or ↓ */
    atomic_bool key_fire;      /* Space */
    atomic_bool key_escape;    /* ESC */
    
    /* Input Thread制御 */
    atomic_bool running;
    HANDLE thread_handle;
} InputState;

/* 入力状態初期化 */
void input_init(InputState *input);

/* Input Thread開始 */
bool input_start_thread(InputState *input);

/* Input Thread停止・待機 */
void input_stop_thread(InputState *input);

/* 入力状態を読み込む */
void input_read_state(InputState *input,
                      bool *key_left, bool *key_right,
                      bool *key_up, bool *key_down,
                      bool *key_fire, bool *key_escape);

#endif /* INPUT_H */

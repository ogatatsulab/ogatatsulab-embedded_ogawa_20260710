#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int64_t frequency;
    int64_t last_time;
    float delta_time;
    float target_fps;
} Timer;

/* タイマーを初期化 */
bool timer_init(Timer *timer, float target_fps);

/* フレーム開始時に呼び出す */
void timer_frame_start(Timer *timer);

/* フレーム終了時に呼び出す（FPS制御） */
void timer_frame_end(Timer *timer);

/* delta timeを取得 */
float timer_get_delta_time(Timer *timer);

#endif /* TIMER_H */

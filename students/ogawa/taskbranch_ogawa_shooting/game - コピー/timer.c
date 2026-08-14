#include "timer.h"
#include <windows.h>

bool timer_init(Timer *timer, float target_fps)
{
    LARGE_INTEGER freq;
    if (!QueryPerformanceFrequency(&freq)) {
        return false;
    }
    
    timer->frequency = freq.QuadPart;
    timer->target_fps = target_fps;
    timer->delta_time = 0.0f;
    
    LARGE_INTEGER current;
    QueryPerformanceCounter(&current);
    timer->last_time = current.QuadPart;
    
    return true;
}

void timer_frame_start(Timer *timer)
{
    LARGE_INTEGER current;
    QueryPerformanceCounter(&current);
    
    int64_t elapsed = current.QuadPart - timer->last_time;
    timer->delta_time = (float)elapsed / (float)timer->frequency;
    
    timer->last_time = current.QuadPart;
}

void timer_frame_end(Timer *timer)
{
    /* フレーム処理時間を計測 */
    LARGE_INTEGER current;
    QueryPerformanceCounter(&current);
    
    float target_frame_time = 1.0f / timer->target_fps;
    float elapsed = (float)(current.QuadPart - timer->last_time) / (float)timer->frequency;
    
    /* 必要な待機時間を計算 */
    float sleep_time = target_frame_time - elapsed;
    
    if (sleep_time > 0.0f) {
        /* ミリ秒に変換して待機 */
        DWORD sleep_ms = (DWORD)(sleep_time * 1000.0f);
        if (sleep_ms > 0) {
            Sleep(sleep_ms);
        }
    }
}

float timer_get_delta_time(Timer *timer)
{
    return timer->delta_time;
}

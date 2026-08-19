#include "game.h"
#include "input.h"
#include "render.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static double counter_seconds(void)
{
    static LARGE_INTEGER frequency;
    LARGE_INTEGER counter;

    if (frequency.QuadPart == 0) QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / (double)frequency.QuadPart;
}

int main(void)
{
    GameState game;
    InputContext input;
    HANDLE input_thread;
    double previous_time;

    srand((unsigned)time(NULL));
    console_init();
    game_init(&game);
    input_init(&input);
    input_thread = input_start(&input);
    if (input_thread == NULL) {
        console_cleanup();
        fprintf(stderr, "入力スレッドを作成できませんでした。\n");
        input_destroy(&input);
        return 1;
    }

    previous_time = counter_seconds();
    while (game.running) {
        double now = counter_seconds();
        double delta_seconds = now - previous_time;
        InputState frame_input = input_take(&input);
        previous_time = now;

        if (delta_seconds > 0.25) delta_seconds = 0.25;
        game_update(&game, &frame_input, delta_seconds);
        render_game(&game);

        {
            double frame_end = counter_seconds();
            double remaining_ms = FRAME_TIME_MS - (frame_end - now) * 1000.0;
            if (remaining_ms > 1.0) Sleep((DWORD)remaining_ms);
        }
    }

    input_stop(&input);
    WaitForSingleObject(input_thread, INFINITE);
    CloseHandle(input_thread);
    input_destroy(&input);

    console_cleanup();
    printf("Game Over. Final score: %d\n", game.score);
    return 0;
}
#include "input.h"

#include <conio.h>

static DWORD WINAPI input_thread_main(void *argument)
{
    InputContext *input = (InputContext *)argument;

    while (InterlockedCompareExchange(&input->stop_requested, 0, 0) == 0) {
        if (_kbhit()) {
            int key = _getch();

            if (key == 0 || key == 224) {
                int special_key = _getch();
                EnterCriticalSection(&input->lock);
                if (special_key == 75) input->state.left = 1;
                if (special_key == 77) input->state.right = 1;
                LeaveCriticalSection(&input->lock);
            } else {
                EnterCriticalSection(&input->lock);
                if (key == 'a' || key == 'A') input->state.left = 1;
                if (key == 'd' || key == 'D') input->state.right = 1;
                if (key == ' ') input->state.fire = 1;
                if (key == 'q' || key == 'Q' || key == 27) input->state.quit = 1;
                LeaveCriticalSection(&input->lock);
            }
        } else {
            Sleep(5);
        }
    }

    return 0;
}

void input_init(InputContext *input)
{
    input->state.left = 0;
    input->state.right = 0;
    input->state.fire = 0;
    input->state.quit = 0;
    input->stop_requested = 0;
    InitializeCriticalSection(&input->lock);
}

HANDLE input_start(InputContext *input)
{
    return CreateThread(NULL, 0, input_thread_main, input, 0, NULL);
}

InputState input_take(InputContext *input)
{
    InputState copy;

    /* 共有するのは入力イベントだけなので、短時間だけロックする。 */
    EnterCriticalSection(&input->lock);
    copy = input->state;
    input->state.left = 0;
    input->state.right = 0;
    input->state.fire = 0;
    LeaveCriticalSection(&input->lock);
    return copy;
}

void input_stop(InputContext *input)
{
    InterlockedExchange(&input->stop_requested, 1);
}

void input_destroy(InputContext *input)
{
    DeleteCriticalSection(&input->lock);
}
#include "input.h"
#include <windows.h>
#include <string.h>

void input_init(InputState *input)
{
    memset(input, 0, sizeof(InputState));
    atomic_init(&input->key_left, false);
    atomic_init(&input->key_right, false);
    atomic_init(&input->key_up, false);
    atomic_init(&input->key_down, false);
    atomic_init(&input->key_fire, false);
    atomic_init(&input->key_escape, false);
    atomic_init(&input->running, false);
    input->thread_handle = NULL;
}

/* Input Thread関数 */
static DWORD WINAPI input_thread_func(LPVOID arg)
{
    InputState *input = (InputState *)arg;
    
    while (atomic_load(&input->running)) {
        /* キーボード状態を読み込む */
        /* GetAsyncKeyState: 0x8000 = キーが押されている */
        
        bool left = (GetAsyncKeyState('A') & 0x8000) || (GetAsyncKeyState(VK_LEFT) & 0x8000);
        bool right = (GetAsyncKeyState('D') & 0x8000) || (GetAsyncKeyState(VK_RIGHT) & 0x8000);
        bool up = (GetAsyncKeyState('W') & 0x8000) || (GetAsyncKeyState(VK_UP) & 0x8000);
        bool down = (GetAsyncKeyState('S') & 0x8000) || (GetAsyncKeyState(VK_DOWN) & 0x8000);
        bool fire = (GetAsyncKeyState(VK_SPACE) & 0x8000);
        bool escape = (GetAsyncKeyState(VK_ESCAPE) & 0x8000);
        
        /* atomic変数に書き込む */
        atomic_store(&input->key_left, left);
        atomic_store(&input->key_right, right);
        atomic_store(&input->key_up, up);
        atomic_store(&input->key_down, down);
        atomic_store(&input->key_fire, fire);
        atomic_store(&input->key_escape, escape);
        
        /* CPU使用率を抑えるため少し待機 */
        Sleep(10);
    }
    
    return 0;
}

bool input_start_thread(InputState *input)
{
    atomic_store(&input->running, true);
    
    input->thread_handle = CreateThread(
        NULL,                   /* セキュリティ属性 */
        0,                      /* スタックサイズ */
        input_thread_func,      /* スレッド関数 */
        input,                  /* 引数 */
        0,                      /* 作成フラグ */
        NULL                    /* スレッドID */
    );
    
    if (input->thread_handle == NULL) {
        atomic_store(&input->running, false);
        return false;
    }
    
    return true;
}

void input_stop_thread(InputState *input)
{
    atomic_store(&input->running, false);
    
    if (input->thread_handle != NULL) {
        WaitForSingleObject(input->thread_handle, INFINITE);
        CloseHandle(input->thread_handle);
        input->thread_handle = NULL;
    }
}

void input_read_state(InputState *input,
                      bool *key_left, bool *key_right,
                      bool *key_up, bool *key_down,
                      bool *key_fire, bool *key_escape)
{
    *key_left = atomic_load(&input->key_left);
    *key_right = atomic_load(&input->key_right);
    *key_up = atomic_load(&input->key_up);
    *key_down = atomic_load(&input->key_down);
    *key_fire = atomic_load(&input->key_fire);
    *key_escape = atomic_load(&input->key_escape);
}

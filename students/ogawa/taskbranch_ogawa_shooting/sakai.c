// simple console shooter for Windows (MinGW-w64 GCC)
// Compile: gcc -std=c17 -Wall -Wextra -O2 main2.c -o shooter.exe

#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Configuration
#define TARGET_FPS      60
// Number of frames between enemy moves. Larger -> slower movement.
#define ENEMY_SPEED     8
// Number of frames between missile moves. Larger -> slower movement.
#define MISSILE_SPEED   2
// Frames between shots when holding fire
#define FIRE_RATE       3
#define GAME_WIDTH      60
#define GAME_HEIGHT     20
#define MAX_MISSILES    32

typedef struct {
    int x, y;
    int active;
} Missile;

typedef struct {
    int x, y;
} Player;

typedef struct {
    int x, y;
    int dir; // +1 right, -1 left
    int active;
} Enemy;

// Shared input state (written by input thread, read by game thread)
static volatile LONG in_left = 0;
static volatile LONG in_right = 0;
// fire hold state: 1 when space is down, 0 when up
static volatile LONG in_fire_hold = 0;
static volatile LONG in_quit = 0;

// Game state (only modified by game thread)
static Player player;
static Enemy enemy;
static Missile missiles[MAX_MISSILES];

static char screen[GAME_HEIGHT][GAME_WIDTH];

static HANDLE hStdin = NULL;
static HANDLE hStdout = NULL;

// Utility: enable VT processing for ANSI escapes
static void enable_ansi(void) {
    DWORD mode = 0;
    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStdout == INVALID_HANDLE_VALUE) return;
    if (!GetConsoleMode(hStdout, &mode)) return;
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hStdout, mode);
}

// Clear screen and hide cursor using ANSI
static void console_init(void) {
    enable_ansi();
    printf("\x1b[2J"); // clear
    printf("\x1b[?25l"); // hide cursor
    printf("\x1b[H"); // move to home
    fflush(stdout);
}

static void console_restore(void) {
    printf("\x1b[?25h"); // show cursor
    printf("\x1b[0m\n");
    fflush(stdout);
}

// Input thread: reads console input events and updates shared flags
DWORD WINAPI input_thread_proc(LPVOID lpParam) {
    (void)lpParam;
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hStdin, &mode);
    SetConsoleMode(hStdin, mode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT));

    // We read events in batches to avoid input backlog.
    while (1) {
        // Block until at least one event available
        INPUT_RECORD rec;
        DWORD read = 0;
        if (!ReadConsoleInputA(hStdin, &rec, 1, &read)) break;
        // process first
        if (rec.EventType == KEY_EVENT) {
            KEY_EVENT_RECORD k = rec.Event.KeyEvent;
            if (k.bKeyDown) {
                switch (k.wVirtualKeyCode) {
                    case VK_LEFT:
                        InterlockedExchange(&in_left, 1);
                        break;
                    case VK_RIGHT:
                        InterlockedExchange(&in_right, 1);
                        break;
                    case VK_SPACE:
                        InterlockedExchange(&in_fire_hold, 1);
                        break;
                    case VK_ESCAPE:
                        InterlockedExchange(&in_quit, 1);
                        return 0;
                    default:
                        if (k.uChar.AsciiChar == 'q' || k.uChar.AsciiChar == 'Q') {
                            InterlockedExchange(&in_quit, 1);
                            return 0;
                        }
                        break;
                }
            } else {
                switch (k.wVirtualKeyCode) {
                    case VK_LEFT:
                        InterlockedExchange(&in_left, 0);
                        break;
                    case VK_RIGHT:
                        InterlockedExchange(&in_right, 0);
                        break;
                    case VK_SPACE:
                        InterlockedExchange(&in_fire_hold, 0);
                        break;
                    default:
                        break;
                }
            }
        }

        // Drain any remaining events currently in the buffer to avoid backlog
        DWORD pending = 0;
        if (GetNumberOfConsoleInputEvents(hStdin, &pending) && pending > 0) {
            const DWORD MAXD = 128;
            DWORD toRead = (pending > MAXD) ? MAXD : pending;
            INPUT_RECORD recs[MAXD];
            DWORD got = 0;
            if (ReadConsoleInputA(hStdin, recs, toRead, &got)) {
                for (DWORD i = 0; i < got; ++i) {
                    if (recs[i].EventType != KEY_EVENT) continue;
                    KEY_EVENT_RECORD k = recs[i].Event.KeyEvent;
                    if (k.bKeyDown) {
                        switch (k.wVirtualKeyCode) {
                            case VK_LEFT: InterlockedExchange(&in_left, 1); break;
                            case VK_RIGHT: InterlockedExchange(&in_right, 1); break;
                            case VK_SPACE: InterlockedExchange(&in_fire_hold, 1); break;
                            case VK_ESCAPE: InterlockedExchange(&in_quit, 1); return 0;
                            default: if (k.uChar.AsciiChar == 'q' || k.uChar.AsciiChar == 'Q') { InterlockedExchange(&in_quit, 1); return 0; } break;
                        }
                    } else {
                        switch (k.wVirtualKeyCode) {
                            case VK_LEFT: InterlockedExchange(&in_left, 0); break;
                            case VK_RIGHT: InterlockedExchange(&in_right, 0); break;
                            case VK_SPACE: InterlockedExchange(&in_fire_hold, 0); break;
                            default: break;
                        }
                    }
                }
            }
        }
    }
    return 0;
}

// Screen buffer helpers
static void clear_screen_buffer(void) {
    for (int y = 0; y < GAME_HEIGHT; ++y)
        for (int x = 0; x < GAME_WIDTH; ++x)
            screen[y][x] = ' ';
}

static void draw_to_console(void) {
    // Move cursor to top-left and output buffer all at once
    printf("\x1b[H");
    for (int y = 0; y < GAME_HEIGHT; ++y) {
        fwrite(screen[y], 1, GAME_WIDTH, stdout);
        putchar('\n');
    }
    fflush(stdout);
}

// Missile management
static void spawn_missile(int x, int y) {
    for (int i = 0; i < MAX_MISSILES; ++i) {
        if (!missiles[i].active) {
            missiles[i].active = 1;
            missiles[i].x = x;
            missiles[i].y = y - 1; // appear above player
            return;
        }
    }
}

int main(void) {
    // init
    memset(missiles, 0, sizeof(missiles));
    player.x = GAME_WIDTH/2;
    player.y = GAME_HEIGHT-1;

    enemy.x = 2;
    enemy.y = 2;
    enemy.dir = 1;
    enemy.active = 1;

    console_init();

    // start input thread
    HANDLE hInputThread = CreateThread(NULL, 0, input_thread_proc, NULL, 0, NULL);
    if (!hInputThread) {
        console_restore();
        fprintf(stderr, "Failed to create input thread\n");
        return 1;
    }

    // Timing
    LARGE_INTEGER freq, t0, t1;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&t0);
    double target_frame_time = 1.0 / TARGET_FPS;

    int frame = 0;
    int enemy_tick = 0;
    int missile_tick = 0;
    int fire_cool = 0;

    int game_running = 1;
    int game_won = 0;

    while (game_running) {
        QueryPerformanceCounter(&t1);
        double elapsed = (double)(t1.QuadPart - t0.QuadPart) / (double)freq.QuadPart;
        if (elapsed < target_frame_time) {
            // sleep a bit then continue to next iteration (busy wait minimal)
            DWORD ms = (DWORD)((target_frame_time - elapsed) * 1000.0);
            if (ms > 0) Sleep(ms);
            QueryPerformanceCounter(&t1);
        }
        t0 = t1;

        // Input consumption (no heavy locking; read atomic flags)
        if (InterlockedCompareExchange(&in_quit, 0, 0)) {
            break;
        }

        if (InterlockedCompareExchange(&in_left, 0, 0)) {
            if (player.x > 0) player.x -= 1;
        }
        if (InterlockedCompareExchange(&in_right, 0, 0)) {
            if (player.x < GAME_WIDTH-1) player.x += 1;
        }

        // Fire handling: continuous while holding space, controlled by FIRE_RATE
        if (fire_cool > 0) fire_cool--;
        if (InterlockedCompareExchange(&in_fire_hold, 0, 0)) {
            if (fire_cool == 0) {
                spawn_missile(player.x, player.y);
                fire_cool = FIRE_RATE;
            }
        }

        // Update enemy based on ENEMY_SPEED frames
        enemy_tick++;
        if (enemy_tick >= ENEMY_SPEED) {
            enemy_tick = 0;
            if (enemy.active) {
                enemy.x += enemy.dir;
                if (enemy.x <= 0) { enemy.x = 0; enemy.dir = 1; }
                if (enemy.x >= GAME_WIDTH-1) { enemy.x = GAME_WIDTH-1; enemy.dir = -1; }
            }
        }

        // Update missiles based on MISSILE_SPEED frames
        missile_tick++;
        if (missile_tick >= MISSILE_SPEED) {
            missile_tick = 0;
            for (int i = 0; i < MAX_MISSILES; ++i) {
                if (missiles[i].active) {
                    missiles[i].y -= 1;
                    if (missiles[i].y < 0) missiles[i].active = 0;
                }
            }
        }

        // Collision detection (missiles vs enemy)
        if (enemy.active) {
            for (int i = 0; i < MAX_MISSILES; ++i) {
                if (missiles[i].active) {
                    if (missiles[i].x == enemy.x && missiles[i].y == enemy.y) {
                        missiles[i].active = 0;
                        enemy.active = 0;
                        game_won = 1;
                        game_running = 0;
                        break;
                    }
                }
            }
        }

        // Build screen buffer
        clear_screen_buffer();
        if (enemy.active) screen[enemy.y][enemy.x] = '@';
        for (int i = 0; i < MAX_MISSILES; ++i) {
            if (missiles[i].active) {
                // bounds check
                if (missiles[i].y >= 0 && missiles[i].y < GAME_HEIGHT && missiles[i].x >=0 && missiles[i].x < GAME_WIDTH)
                    screen[missiles[i].y][missiles[i].x] = '|';
            }
        }
        // Player
        screen[player.y][player.x] = 'M';

        // Draw
        draw_to_console();

        frame++;
    }

    // Final screen: show win/quit message
    clear_screen_buffer();
    for (int y = 0; y < GAME_HEIGHT; ++y) for (int x = 0; x < GAME_WIDTH; ++x) screen[y][x] = ' ';
    const char *msg = game_won ? "GAME CLEAR! Press any key to exit." : "QUIT. Press any key to exit.";
    int mx = (GAME_WIDTH - (int)strlen(msg)) / 2;
    int my = GAME_HEIGHT/2;
    if (mx < 0) mx = 0;
    for (size_t i = 0; i < strlen(msg) && mx + (int)i < GAME_WIDTH; ++i)
        screen[my][mx + i] = msg[i];
    draw_to_console();

    // wait for a key press (non-blocking input thread is running; but we can block here on ReadConsoleInput)
    // restore console mode for input reading
    // Signal input thread to stop if still running
    InterlockedExchange(&in_quit, 1);

    // wait for input thread to finish
    WaitForSingleObject(hInputThread, INFINITE);
    CloseHandle(hInputThread);

    // Wait for any key to be pressed to exit
    // restore console cursor
    console_restore();

    return 0;
}

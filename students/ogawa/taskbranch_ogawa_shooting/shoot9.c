/*
 * shoot9.c
 * Windows + MinGW GCC 向けの最小コンソールシューティングゲーム
 */

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define SCREEN_WIDTH 40
#define SCREEN_HEIGHT 20
#define FPS 10
#define FRAME_TIME_MS (1000 / FPS)

#define PLAYER_SPEED 1
#define ENEMY_SPEED 1
#define ENEMY_MOVE_INTERVAL 4
#define MISSILE_SPEED 1
#define MISSILE_MOVE_INTERVAL 1

#define FRAME_WIDTH (SCREEN_WIDTH + 2)
#define FRAME_HEIGHT (SCREEN_HEIGHT + 2)
#define SCORE_PER_ENEMY 10
#define INITIAL_LIVES 3
#define NORMAL_ENEMIES_TO_BOSS 3
#define BOSS_HIT_POINTS 4
#define BOSS_SIZE 2
#define HIT_EFFECT_FRAMES 3
#define MAX_MISSILES 6

#define PLAYER_CHAR '^'
#define ENEMY_CHAR 'W'
#define MISSILE_CHAR '|'
#define BORDER_CHAR '#'

/* 入力スレッドからメインスレッドへ渡すイベントだけを共有する。 */
typedef struct {
    int x;
    int y;
    int active;
    int wait_frames;
} Missile;

typedef struct {
    int left;
    int right;
    int fire;
    int quit;
} InputEvents;

typedef struct {
    CRITICAL_SECTION lock;
    InputEvents events;
    volatile LONG stop_requested;
} InputContext;

typedef struct {
    int player_x;
    int player_y;
    int enemy_x;
    int enemy_y;
    int enemy_active;
    int enemy_wait_frames;
    int boss_active;
    int boss_hit_points;
    int defeated_enemies;
    Missile missiles[MAX_MISSILES];
    int hit_effect_x;
    int hit_effect_y;
    int hit_effect_frames;
    int score;
    int lives;
    int running;
    int game_over;
} GameState;

static char screen[FRAME_HEIGHT][FRAME_WIDTH];

static int clamp_x(int x)
{
    if (x < 0) return 0;
    if (x >= SCREEN_WIDTH) return SCREEN_WIDTH - 1;
    return x;
}

static DWORD WINAPI input_thread_main(void *argument)
{
    InputContext *input = (InputContext *)argument;
    int previous_space_down = 0;

    while (InterlockedCompareExchange(&input->stop_requested, 0, 0) == 0) {
        InputEvents current = {0};
        int key_count = 0;
        int space_down;

        /* 移動はキー状態を直接読むため、コンソールの移動入力を蓄積しない。 */
        current.left = (GetAsyncKeyState(VK_LEFT) & 0x8000) != 0 ||
                       (GetAsyncKeyState('A') & 0x8000) != 0;
        current.right = (GetAsyncKeyState(VK_RIGHT) & 0x8000) != 0 ||
                        (GetAsyncKeyState('D') & 0x8000) != 0;
        space_down = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;
        current.fire = space_down && !previous_space_down;
        previous_space_down = space_down;

        /* 単発キーだけをキューから最大64個まで消費する。 */
        while (_kbhit() && key_count < 64) {
            int key = _getch();
            ++key_count;

            if (key == 0 || key == 224) {
                if (_kbhit()) (void)_getch();
            } else if (key == 'q' || key == 'Q' || key == 27) {
                current.quit = 1;
            }
        }

        EnterCriticalSection(&input->lock);
        input->events.left = current.left;
        input->events.right = current.right;
        input->events.fire |= current.fire;
        input->events.quit |= current.quit;
        LeaveCriticalSection(&input->lock);

        Sleep(5);
    }

    return 0;
}

static void input_init(InputContext *input)
{
    input->events.left = 0;
    input->events.right = 0;
    input->events.fire = 0;
    input->events.quit = 0;
    input->stop_requested = 0;
    InitializeCriticalSection(&input->lock);
}

static HANDLE input_start(InputContext *input)
{
    return CreateThread(NULL, 0, input_thread_main, input, 0, NULL);
}

static InputEvents input_take(InputContext *input)
{
    InputEvents events;

    EnterCriticalSection(&input->lock);
    events = input->events;
    input->events.left = 0;
    input->events.right = 0;
    input->events.fire = 0;
    input->events.quit = 0;
    LeaveCriticalSection(&input->lock);
    return events;
}

static void input_stop(InputContext *input)
{
    InterlockedExchange(&input->stop_requested, 1);
}

static void input_destroy(InputContext *input)
{
    DeleteCriticalSection(&input->lock);
}

static void game_init(GameState *game)
{
    game->player_x = SCREEN_WIDTH / 2;
    game->player_y = SCREEN_HEIGHT - 1;
    game->enemy_x = SCREEN_WIDTH / 2;
    game->enemy_y = 0;
    game->enemy_active = 1;
    game->enemy_wait_frames = 0;
    game->boss_active = 0;
    game->boss_hit_points = 0;
    game->defeated_enemies = 0;
    game->hit_effect_x = 0;
    game->hit_effect_y = 0;
    game->hit_effect_frames = 0;
    game->score = 0;
    game->lives = INITIAL_LIVES;
    game->running = 1;
    game->game_over = 0;
}

static void respawn_enemy(GameState *game)
{
    game->boss_active = 0;
    game->boss_hit_points = 0;
    game->enemy_x = rand() % SCREEN_WIDTH;
    game->enemy_y = 0;
    game->enemy_active = 1;
    game->enemy_wait_frames = 0;
}

static void start_boss(GameState *game)
{
    game->boss_active = 1;
    game->boss_hit_points = BOSS_HIT_POINTS;
    game->enemy_x = (SCREEN_WIDTH - BOSS_SIZE) / 2;
    game->enemy_y = 0;
    game->enemy_active = 1;
    game->enemy_wait_frames = 0;
}

static void lose_life(GameState *game)
{
    --game->lives;
    if (game->lives <= 0) {
        game->lives = 0;
        game->running = 0;
        game->game_over = 1;
    } else {
        if (game->boss_active) {
            game->enemy_x = (SCREEN_WIDTH - BOSS_SIZE) / 2;
            game->enemy_y = 0;
            game->enemy_active = 1;
            game->enemy_wait_frames = 0;
        } else {
            respawn_enemy(game);
        }
    }
}

static void fire_missile(GameState *game)
{
    int index;

    for (index = 0; index < MAX_MISSILES; ++index) {
        Missile *missile = &game->missiles[index];
        if (!missile->active) {
            missile->x = game->player_x;
            missile->y = game->player_y - 1;
            missile->active = 1;
            missile->wait_frames = 0;
            return;
        }
    }
}

static void check_collisions(GameState *game)
{
    int index;

    for (index = 0; index < MAX_MISSILES; ++index) {
        Missile *missile = &game->missiles[index];
        int hit;

        if (!game->enemy_active || !missile->active) continue;

        hit = game->enemy_x == missile->x &&
              abs(game->enemy_y - missile->y) <= 1;

        if (game->boss_active) {
            hit = missile->x >= game->enemy_x &&
                  missile->x < game->enemy_x + BOSS_SIZE &&
                  missile->y >= game->enemy_y &&
                  missile->y < game->enemy_y + BOSS_SIZE;
        }

        if (hit) {
            missile->active = 0;
            game->hit_effect_x = missile->x;
            game->hit_effect_y = missile->y;
            game->hit_effect_frames = HIT_EFFECT_FRAMES;
            if (game->boss_active) {
                --game->boss_hit_points;
                if (game->boss_hit_points <= 0) {
                    game->enemy_active = 0;
                    game->boss_active = 0;
                    game->score += SCORE_PER_ENEMY * BOSS_HIT_POINTS;
                    game->defeated_enemies = 0;
                    respawn_enemy(game);
                }
            } else {
                game->enemy_active = 0;
                game->score += SCORE_PER_ENEMY;
                ++game->defeated_enemies;

                if (game->defeated_enemies >= NORMAL_ENEMIES_TO_BOSS) {
                    start_boss(game);
                } else {
                    respawn_enemy(game);
                }
            }
        }
    }

    if (game->enemy_active && game->player_x >= game->enemy_x &&
        game->player_x < game->enemy_x + (game->boss_active ? BOSS_SIZE : 1) &&
        game->player_y >= game->enemy_y &&
        game->player_y < game->enemy_y + (game->boss_active ? BOSS_SIZE : 1)) {
        lose_life(game);
    }
}

static void update_game(GameState *game, InputEvents events)
{
    if (events.quit) {
        game->running = 0;
        return;
    }

    if (game->hit_effect_frames > 0) {
        --game->hit_effect_frames;
    }

    if (events.left) game->player_x -= PLAYER_SPEED;
    if (events.right) game->player_x += PLAYER_SPEED;
    game->player_x = clamp_x(game->player_x);

    if (events.fire) fire_missile(game);

    if (game->enemy_active) {
        ++game->enemy_wait_frames;
        if (game->enemy_wait_frames >= ENEMY_MOVE_INTERVAL) {
            game->enemy_y += ENEMY_SPEED;
            game->enemy_wait_frames = 0;
            if (game->enemy_y >= SCREEN_HEIGHT) {
                lose_life(game);
            }
        }
    }

    {
        int index;
        for (index = 0; index < MAX_MISSILES; ++index) {
            Missile *missile = &game->missiles[index];
            if (!missile->active) continue;

            ++missile->wait_frames;
            if (missile->wait_frames >= MISSILE_MOVE_INTERVAL) {
                missile->y -= MISSILE_SPEED;
                missile->wait_frames = 0;
                if (missile->y < 0) missile->active = 0;
            }
        }
    }

    check_collisions(game);
}

static void clear_screen_buffer(void)
{
    int y;
    int x;

    for (y = 0; y < FRAME_HEIGHT; ++y) {
        for (x = 0; x < FRAME_WIDTH; ++x) {
            screen[y][x] = ' ';
        }
    }
}

static void draw_frame(void)
{
    int y;
    int x;

    for (x = 0; x < FRAME_WIDTH; ++x) {
        screen[0][x] = BORDER_CHAR;
        screen[FRAME_HEIGHT - 1][x] = BORDER_CHAR;
    }
    for (y = 0; y < FRAME_HEIGHT; ++y) {
        screen[y][0] = BORDER_CHAR;
        screen[y][FRAME_WIDTH - 1] = BORDER_CHAR;
    }
}

static void draw_objects(const GameState *game)
{
    screen[game->player_y + 1][game->player_x + 1] = PLAYER_CHAR;

    if (game->enemy_active) {
        screen[game->enemy_y + 1][game->enemy_x + 1] =
            game->boss_active ? 'B' : ENEMY_CHAR;
        if (game->boss_active) {
            screen[game->enemy_y + 1][game->enemy_x + 2] = 'B';
            screen[game->enemy_y + 2][game->enemy_x + 1] = 'B';
            screen[game->enemy_y + 2][game->enemy_x + 2] = 'B';
        }
    }
    {
        int index;
        for (index = 0; index < MAX_MISSILES; ++index) {
            const Missile *missile = &game->missiles[index];
            if (missile->active && missile->y >= 0 &&
                missile->y < SCREEN_HEIGHT) {
                screen[missile->y + 1][missile->x + 1] = MISSILE_CHAR;
            }
        }
    }

    if (game->hit_effect_frames > 0) {
        static const char effect[3][3] = {
            {' ', '*', ' '},
            {'*', 'X', '*'},
            {' ', '*', ' '}
        };
        int effect_y;
        int effect_x;

        for (effect_y = 0; effect_y < 3; ++effect_y) {
            for (effect_x = 0; effect_x < 3; ++effect_x) {
                int x = game->hit_effect_x + effect_x - 1;
                int y = game->hit_effect_y + effect_y - 1;
                if (effect[effect_y][effect_x] != ' ' &&
                    x >= 0 && x < SCREEN_WIDTH &&
                    y >= 0 && y < SCREEN_HEIGHT) {
                    screen[y + 1][x + 1] = effect[effect_y][effect_x];
                }
            }
        }
    }
}

static void display_screen(const GameState *game)
{
    int y;

    printf("\x1b[H");
    for (y = 0; y < FRAME_HEIGHT; ++y) {
        printf("%.*s\n", FRAME_WIDTH, screen[y]);
    }
    if (game->boss_active) {
        printf("Score: %d   Lives: %d   BOSS HP: %d   A/D or arrows: move   Space: fire   Q: quit\n",
               game->score, game->lives, game->boss_hit_points);
    } else {
        printf("Score: %d   Lives: %d   A/D or arrows: move   Space: fire   Q: quit\n",
               game->score, game->lives);
    }
    fflush(stdout);
}

static void enable_vt_mode(DWORD *old_mode, HANDLE *console)
{
    DWORD mode;

    *console = GetStdHandle(STD_OUTPUT_HANDLE);
    if (*console == INVALID_HANDLE_VALUE || !GetConsoleMode(*console, &mode)) {
        *console = NULL;
        return;
    }

    *old_mode = mode;
    SetConsoleMode(*console, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

static void restore_console(DWORD old_mode, HANDLE console)
{
    printf("\x1b[?25h\x1b[0m\n");
    fflush(stdout);
    if (console != NULL) SetConsoleMode(console, old_mode);
}

int main(void)
{
    GameState game = {0};
    InputContext input;
    HANDLE input_thread;
    DWORD old_mode = 0;
    HANDLE console;

    srand((unsigned int)GetTickCount());
    game_init(&game);
    input_init(&input);
    input_thread = input_start(&input);
    if (input_thread == NULL) {
        input_destroy(&input);
        fprintf(stderr, "入力スレッドを開始できませんでした。\n");
        return EXIT_FAILURE;
    }

    enable_vt_mode(&old_mode, &console);
    printf("\x1b[2J\x1b[?25l");
    fflush(stdout);

    while (game.running) {
        InputEvents events = input_take(&input);

        update_game(&game, events);
        clear_screen_buffer();
        draw_frame();
        draw_objects(&game);
        display_screen(&game);
        Sleep(FRAME_TIME_MS);
    }

    input_stop(&input);
    WaitForSingleObject(input_thread, INFINITE);
    CloseHandle(input_thread);
    input_destroy(&input);

    if (game.game_over) {
        printf("GAME OVER! Score: %d\n", game.score);
    } else {
        printf("Game ended. Score: %d\n", game.score);
    }
    restore_console(old_mode, console);
    return EXIT_SUCCESS;
}

/*
 * Windows GCC でのコンパイル例:
 * gcc -std=c11 -Wall -Wextra -O2 -o shoot9.exe shoot9.c
 * 実行例:
 * .\shoot9.exe
 */

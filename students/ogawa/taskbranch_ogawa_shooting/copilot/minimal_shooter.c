#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define WIDTH 80
#define HEIGHT 25
#define FPS 20
#define FRAME_TIME_MS (1000 / FPS)

#define PLAYER_SPEED 18.0
#define MISSILE_SPEED 12.0
#define ENEMY_SPEED 1.0
#define SCORE_PER_ENEMY 10

typedef struct {
    double x;
    int y;
} Player;

typedef struct {
    double x;
    double y;
    int active;
} Enemy;

typedef struct {
    double x;
    double y;
    int active;
} Missile;

typedef struct {
    int left;
    int right;
    int fire;
    int quit;
} InputState;

typedef struct {
    CRITICAL_SECTION lock;
    InputState state;
    LONG stop_requested;
} InputContext;

typedef struct {
    Player player;
    Enemy enemy;
    Missile missile;
    int score;
    int running;
} GameState;

static char screen[HEIGHT][WIDTH];
static HANDLE output_handle = INVALID_HANDLE_VALUE;
static DWORD original_output_mode;
static int saved_output_mode;

static double now_seconds(void)
{
    static LARGE_INTEGER frequency;
    LARGE_INTEGER counter;

    if (frequency.QuadPart == 0) {
        QueryPerformanceFrequency(&frequency);
    }
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / (double)frequency.QuadPart;
}

static int rounded_position(double position)
{
    return (int)(position + 0.5);
}

static void put_char(int x, int y, char value)
{
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
        screen[y][x] = value;
    }
}

static void clear_screen_buffer(void)
{
    memset(screen, ' ', sizeof(screen));
}

static void init_console(void)
{
    DWORD mode;

    output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (output_handle != INVALID_HANDLE_VALUE &&
        GetConsoleMode(output_handle, &mode)) {
        original_output_mode = mode;
        saved_output_mode = 1;
        SetConsoleMode(output_handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }

    /* ANSIは画面消去、カーソル移動、カーソル表示だけに使う。 */
    fputs("\x1b[2J\x1b[H\x1b[?25l", stdout);
    fflush(stdout);
}

static void cleanup_console(void)
{
    fputs("\x1b[0m\x1b[?25h\x1b[H\n", stdout);
    fflush(stdout);

    if (saved_output_mode) {
        SetConsoleMode(output_handle, original_output_mode);
    }
}

static void init_game(GameState *game)
{
    memset(game, 0, sizeof(*game));
    game->player.x = WIDTH / 2;
    game->player.y = HEIGHT - 2;
    game->enemy.x = 1 + rand() % (WIDTH - 2);
    game->enemy.y = 1.0;
    game->enemy.active = 1;
    game->running = 1;
}

static void update_player(GameState *game, const InputState *input,
                          double delta_seconds)
{
    int direction = input->right - input->left;
    double movement = direction * PLAYER_SPEED * delta_seconds;

    game->player.x += movement;
    if (game->player.x < 0.0) game->player.x = 0.0;
    if (game->player.x > WIDTH - 1) game->player.x = WIDTH - 1;
}

static void fire_missile(GameState *game)
{
    if (!game->missile.active) {
        game->missile.x = game->player.x;
        game->missile.y = game->player.y - 1;
        game->missile.active = 1;
    }
}

static void update_missile(GameState *game, double delta_seconds)
{
    if (!game->missile.active) return;

    game->missile.y -= MISSILE_SPEED * delta_seconds;
    if (game->missile.y < 1.0) {
        game->missile.active = 0;
    }
}

static void update_enemy(GameState *game, double delta_seconds)
{
    if (!game->enemy.active) return;

    game->enemy.y += ENEMY_SPEED * delta_seconds;
    if (game->enemy.y >= HEIGHT - 1) {
        game->enemy.active = 0;
        game->running = 0;
    }
}

static void check_collisions(GameState *game)
{
    int missile_x;
    int missile_y;
    int enemy_x;
    int enemy_y;

    if (!game->missile.active || !game->enemy.active) return;

    missile_x = rounded_position(game->missile.x);
    missile_y = rounded_position(game->missile.y);
    enemy_x = rounded_position(game->enemy.x);
    enemy_y = rounded_position(game->enemy.y);

    if (abs(missile_x - enemy_x) <= 1 && abs(missile_y - enemy_y) <= 1) {
        game->missile.active = 0;
        game->enemy.x = 1 + rand() % (WIDTH - 2);
        game->enemy.y = 1.0;
        game->score += SCORE_PER_ENEMY;
    }
}

static void update_game(GameState *game, const InputState *input,
                        double delta_seconds)
{
    if (input->quit) {
        game->running = 0;
        return;
    }

    update_player(game, input, delta_seconds);
    if (input->fire) fire_missile(game);
    update_enemy(game, delta_seconds);
    update_missile(game, delta_seconds);
    check_collisions(game);
}

static void build_screen_buffer(const GameState *game)
{
    char status[WIDTH + 1];
    int x;

    clear_screen_buffer();
    snprintf(status, sizeof(status),
             "SCORE: %d  A/D or arrows: MOVE  SPACE: FIRE  ESC: QUIT",
             game->score);
    for (x = 0; status[x] != '\0' && x < WIDTH; ++x) {
        screen[0][x] = status[x];
    }

    if (game->enemy.active) {
        put_char(rounded_position(game->enemy.x),
                 rounded_position(game->enemy.y), 'W');
    }
    if (game->missile.active) {
        put_char(rounded_position(game->missile.x),
                 rounded_position(game->missile.y), '|');
    }
    put_char(rounded_position(game->player.x), game->player.y, '^');
}

static void render_screen(void)
{
    char output[HEIGHT * (WIDTH + 2) + 4];
    size_t length = 0;
    int y;

    output[length++] = '\x1b';
    output[length++] = '[';
    output[length++] = 'H';
    for (y = 0; y < HEIGHT; ++y) {
        memcpy(output + length, screen[y], WIDTH);
        length += WIDTH;
        output[length++] = '\r';
        output[length++] = '\n';
    }
    fwrite(output, 1, length, stdout);
    fflush(stdout);
}

static DWORD WINAPI input_thread(void *argument)
{
    InputContext *input = (InputContext *)argument;

    while (InterlockedCompareExchange(&input->stop_requested, 0, 0) == 0) {
        if (_kbhit()) {
            int key = _getch();

            EnterCriticalSection(&input->lock);
            if (key == 0 || key == 224) {
                int special_key = _getch();
                if (special_key == 75) input->state.left = 1;
                if (special_key == 77) input->state.right = 1;
            } else if (key == 'a' || key == 'A') {
                input->state.left = 1;
            } else if (key == 'd' || key == 'D') {
                input->state.right = 1;
            } else if (key == ' ') {
                input->state.fire = 1;
            } else if (key == 27) {
                input->state.quit = 1;
            }
            LeaveCriticalSection(&input->lock);
        } else {
            Sleep(5);
        }
    }
    return 0;
}

static void init_input(InputContext *input)
{
    memset(input, 0, sizeof(*input));
    InitializeCriticalSection(&input->lock);
}

static InputState take_input(InputContext *input)
{
    InputState current;

    EnterCriticalSection(&input->lock);
    current = input->state;
    input->state.left = 0;
    input->state.right = 0;
    input->state.fire = 0;
    input->state.quit = 0;
    LeaveCriticalSection(&input->lock);
    return current;
}

static void stop_input(InputContext *input, HANDLE thread_handle)
{
    InterlockedExchange(&input->stop_requested, 1);
    WaitForSingleObject(thread_handle, INFINITE);
    CloseHandle(thread_handle);
    DeleteCriticalSection(&input->lock);
}

int main(void)
{
    GameState game;
    InputContext input;
    HANDLE input_handle;
    double previous_time;

    srand((unsigned)time(NULL));
    init_console();
    init_game(&game);
    init_input(&input);

    input_handle = CreateThread(NULL, 0, input_thread, &input, 0, NULL);
    if (input_handle == NULL) {
        cleanup_console();
        DeleteCriticalSection(&input.lock);
        fprintf(stderr, "入力スレッドを作成できませんでした。\n");
        return 1;
    }

    previous_time = now_seconds();
    while (game.running) {
        double frame_start = now_seconds();
        double delta_seconds = frame_start - previous_time;
        InputState frame_input = take_input(&input);
        double frame_elapsed;
        double remaining_ms;

        previous_time = frame_start;
        if (delta_seconds > 0.25) delta_seconds = 0.25;

        update_game(&game, &frame_input, delta_seconds);
        build_screen_buffer(&game);
        render_screen();

        frame_elapsed = now_seconds() - frame_start;
        remaining_ms = FRAME_TIME_MS - frame_elapsed * 1000.0;
        if (remaining_ms > 1.0) Sleep((DWORD)remaining_ms);
    }

    stop_input(&input, input_handle);
    cleanup_console();
    printf("Game Over. Final score: %d\n", game.score);
    return 0;
}

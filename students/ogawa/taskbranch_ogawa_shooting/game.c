/*
 * Windows Console Shooting Game
 * Compiled with MinGW-w64 GCC
 * Design:
 *   - 2 threads: Input thread + Game thread
 *   - Shared data: InputState only (protected by mutex)
 *   - Screen buffer approach (FPS-style rendering)
 *   - Float coordinates internally -> int for screen buffer
 *   - 60 FPS target
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <windows.h>
#include <pthread.h>
#include <time.h>

/* ===== Screen Configuration ===== */
#define SCREEN_WIDTH  80
#define SCREEN_HEIGHT 25
#define TARGET_FPS    60
#define FRAME_TIME_MS (1000.0 / TARGET_FPS)  /* ~16.67 ms */

/* ===== Game Constants ===== */
#define MAX_ENEMIES   32
#define MAX_MISSILES  64

#define PLAYER_CHAR   'A'
#define ENEMY_CHAR    'V'
#define MISSILE_CHAR  '|'
#define BORDER_CHAR   '#'

/* ===== Input State (Shared Data) ===== */
typedef struct {
    bool up;
    bool down;
    bool left;
    bool right;
    bool fire;
    bool quit;
} InputState;

InputState g_input_state = {0};
pthread_mutex_t g_input_mutex = PTHREAD_MUTEX_INITIALIZER;

/* ===== Game Entities ===== */
typedef struct {
    float x, y;
    float vx, vy;
    bool active;
} Player;

typedef struct {
    float x, y;
    float vx, vy;
    bool active;
} Enemy;

typedef struct {
    float x, y;
    float vx, vy;
    bool active;
} Missile;

/* ===== Game State ===== */
typedef struct {
    Player player;
    Enemy enemies[MAX_ENEMIES];
    Missile missiles[MAX_MISSILES];
    
    int score;
    int enemy_count;
    int missile_count;
    bool running;
    
    double frame_time;  /* Delta time in seconds */
} GameState;

/* Screen buffer (global for rendering) */
static char g_screen[SCREEN_HEIGHT][SCREEN_WIDTH];

/* ===== Windows Console Initialization ===== */
static void console_init(void)
{
    /* 
     * Windows console: Enable ANSI escape sequences
     * Requires Windows 10 v1607 or later
     */
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            /* ENABLE_VIRTUAL_TERMINAL_PROCESSING = 0x0004 */
            dwMode |= 0x0004;
            SetConsoleMode(hOut, dwMode);
        }
    }
    
    /* Clear screen and set up */
    printf("\x1b[2J");  /* ANSI: Clear screen */
    printf("\x1b[H");   /* ANSI: Move cursor to (0,0) */
    printf("\x1b[?25l"); /* ANSI: Hide cursor */
    fflush(stdout);
}

static void console_cleanup(void)
{
    /* Show cursor and clear screen on exit */
    printf("\x1b[?25h");  /* ANSI: Show cursor */
    printf("\x1b[2J");    /* ANSI: Clear screen */
    printf("\x1b[H");     /* ANSI: Move cursor to (0,0) */
    fflush(stdout);
}

/* ===== Input Thread ===== */
static void* input_thread_func(void* arg)
{
    (void)arg;
    
    /* Windows Console Input API */
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD dwMode = 0;
    
    /* Save original mode */
    GetConsoleMode(hStdin, &dwMode);
    DWORD dwNewMode = dwMode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
    SetConsoleMode(hStdin, dwNewMode);
    
    /* Read keyboard input continuously */
    while (1) {
        INPUT_RECORD irBuf[32];
        DWORD dwNumRead = 0;
        
        if (!ReadConsoleInput(hStdin, irBuf, 32, &dwNumRead)) {
            break;
        }
        
        for (DWORD i = 0; i < dwNumRead; i++) {
            if (irBuf[i].EventType != KEY_EVENT) continue;
            if (!irBuf[i].Event.KeyEvent.bKeyDown) continue;
            
            WORD vk = irBuf[i].Event.KeyEvent.wVirtualKeyCode;
            
            pthread_mutex_lock(&g_input_mutex);
            
            if (vk == VK_UP || vk == 'W') {
                g_input_state.up = true;
            } else if (vk == VK_DOWN || vk == 'S') {
                g_input_state.down = true;
            } else if (vk == VK_LEFT || vk == 'A') {
                g_input_state.left = true;
            } else if (vk == VK_RIGHT || vk == 'D') {
                g_input_state.right = true;
            } else if (vk == VK_SPACE) {
                g_input_state.fire = true;
            } else if (vk == VK_ESCAPE) {
                g_input_state.quit = true;
            }
            
            pthread_mutex_unlock(&g_input_mutex);
        }
        
        /* Check quit flag */
        pthread_mutex_lock(&g_input_mutex);
        if (g_input_state.quit) {
            pthread_mutex_unlock(&g_input_mutex);
            break;
        }
        pthread_mutex_unlock(&g_input_mutex);
    }
    
    /* Restore console mode */
    SetConsoleMode(hStdin, dwMode);
    
    return NULL;
}

/* ===== Game Logic ===== */
static void game_state_init(GameState* state)
{
    memset(state, 0, sizeof(*state));
    
    /* Initialize player at center-bottom */
    state->player.x = SCREEN_WIDTH / 2.0f;
    state->player.y = SCREEN_HEIGHT - 3.0f;
    state->player.vx = 0.0f;
    state->player.vy = 0.0f;
    state->player.active = true;
    
    state->score = 0;
    state->enemy_count = 0;
    state->missile_count = 0;
    state->running = true;
    state->frame_time = 0.0;
}

static void spawn_enemy(GameState* state)
{
    /* Spawn enemies from top, moving downward */
    static double spawn_timer = 0.0;
    spawn_timer += state->frame_time;
    
    if (spawn_timer < 0.8) {  /* Spawn every 0.8 seconds */
        return;
    }
    spawn_timer = 0.0;
    
    /* Find free enemy slot */
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!state->enemies[i].active) {
            state->enemies[i].x = (rand() % (SCREEN_WIDTH - 4)) + 2.0f;
            state->enemies[i].y = 1.0f;
            state->enemies[i].vx = 0.0f;
            state->enemies[i].vy = 8.0f;  /* Move down */
            state->enemies[i].active = true;
            state->enemy_count++;
            break;
        }
    }
}

static void update_player(GameState* state, const InputState* input)
{
    const float player_speed = 25.0f;  /* pixels/second */
    
    state->player.vx = 0.0f;
    
    if (input->left) {
        state->player.vx = -player_speed;
    }
    if (input->right) {
        state->player.vx = player_speed;
    }
    
    /* Update position */
    state->player.x += state->player.vx * state->frame_time;
    
    /* Boundary check */
    if (state->player.x < 1.0f) state->player.x = 1.0f;
    if (state->player.x > SCREEN_WIDTH - 2.0f) state->player.x = SCREEN_WIDTH - 2.0f;
    
    /* Fire missile */
    if (input->fire) {
        for (int i = 0; i < MAX_MISSILES; i++) {
            if (!state->missiles[i].active) {
                state->missiles[i].x = state->player.x;
                state->missiles[i].y = state->player.y - 1.0f;
                state->missiles[i].vx = 0.0f;
                state->missiles[i].vy = -40.0f;  /* Move up */
                state->missiles[i].active = true;
                state->missile_count++;
                break;
            }
        }
    }
}

static void update_enemies(GameState* state)
{
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!state->enemies[i].active) continue;
        
        state->enemies[i].y += state->enemies[i].vy * state->frame_time;
        
        /* If enemy leaves screen, deactivate */
        if (state->enemies[i].y > SCREEN_HEIGHT) {
            state->enemies[i].active = false;
            state->enemy_count--;
        }
    }
}

static void update_missiles(GameState* state)
{
    for (int i = 0; i < MAX_MISSILES; i++) {
        if (!state->missiles[i].active) continue;
        
        state->missiles[i].y += state->missiles[i].vy * state->frame_time;
        
        /* If missile leaves screen, deactivate */
        if (state->missiles[i].y < 0.0f) {
            state->missiles[i].active = false;
            state->missile_count--;
        }
    }
}

static float distance_sq(float x1, float y1, float x2, float y2)
{
    float dx = x1 - x2;
    float dy = y1 - y2;
    return dx * dx + dy * dy;
}

static void check_collisions(GameState* state)
{
    const float collision_dist = 1.5f;
    const float collision_dist_sq = collision_dist * collision_dist;
    
    for (int i = 0; i < MAX_MISSILES; i++) {
        if (!state->missiles[i].active) continue;
        
        for (int j = 0; j < MAX_ENEMIES; j++) {
            if (!state->enemies[j].active) continue;
            
            float dist_sq = distance_sq(
                state->missiles[i].x, state->missiles[i].y,
                state->enemies[j].x, state->enemies[j].y
            );
            
            if (dist_sq < collision_dist_sq) {
                /* Collision! */
                state->missiles[i].active = false;
                state->missile_count--;
                
                state->enemies[j].active = false;
                state->enemy_count--;
                
                state->score += 10;
                break;
            }
        }
    }
}

/* ===== Rendering ===== */
static void screen_buffer_clear(void)
{
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            g_screen[y][x] = ' ';
        }
    }
}

static void screen_buffer_draw_char(int x, int y, char c)
{
    /* Boundary check */
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        g_screen[y][x] = c;
    }
}

static void screen_buffer_draw_border(void)
{
    /* Top and bottom */
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        g_screen[0][x] = BORDER_CHAR;
        g_screen[SCREEN_HEIGHT - 1][x] = BORDER_CHAR;
    }
    /* Left and right */
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        g_screen[y][0] = BORDER_CHAR;
        g_screen[y][SCREEN_WIDTH - 1] = BORDER_CHAR;
    }
}

static void render_game(const GameState* state)
{
    screen_buffer_clear();
    screen_buffer_draw_border();
    
    /* Draw player */
    if (state->player.active) {
        int px = (int)state->player.x;
        int py = (int)state->player.y;
        screen_buffer_draw_char(px, py, PLAYER_CHAR);
    }
    
    /* Draw enemies */
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (state->enemies[i].active) {
            int ex = (int)state->enemies[i].x;
            int ey = (int)state->enemies[i].y;
            screen_buffer_draw_char(ex, ey, ENEMY_CHAR);
        }
    }
    
    /* Draw missiles */
    for (int i = 0; i < MAX_MISSILES; i++) {
        if (state->missiles[i].active) {
            int mx = (int)state->missiles[i].x;
            int my = (int)state->missiles[i].y;
            screen_buffer_draw_char(mx, my, MISSILE_CHAR);
        }
    }
    
    /* Draw score */
    char score_str[32];
    snprintf(score_str, sizeof(score_str), "Score: %d  Enemies: %d",
             state->score, state->enemy_count);
    for (int x = 0; x < (int)strlen(score_str) && x < SCREEN_WIDTH - 2; x++) {
        g_screen[SCREEN_HEIGHT - 1][x + 1] = score_str[x];
    }
}

static void screen_buffer_output(void)
{
    /* Move cursor to top-left and output entire buffer */
    printf("\x1b[H");  /* ANSI: Move cursor to (0,0) */
    
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        fwrite(g_screen[y], 1, SCREEN_WIDTH, stdout);
        printf("\n");
    }
    fflush(stdout);
}

/* ===== FPS Control ===== */
static double get_time_seconds(void)
{
    static LARGE_INTEGER freq = {0};
    static bool freq_initialized = false;
    
    if (!freq_initialized) {
        QueryPerformanceFrequency(&freq);
        freq_initialized = true;
    }
    
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);
    return (double)count.QuadPart / freq.QuadPart;
}

/* ===== Main Game Loop ===== */
static void game_loop(void)
{
    GameState game_state;
    game_state_init(&game_state);
    
    double prev_time = get_time_seconds();
    
    while (game_state.running) {
        double current_time = get_time_seconds();
        game_state.frame_time = current_time - prev_time;
        prev_time = current_time;
        
        /* Cap delta time to avoid huge jumps */
        if (game_state.frame_time > 0.05) {
            game_state.frame_time = 0.05;
        }
        
        /* ===== Read input (minimal lock) ===== */
        InputState local_input = {0};
        pthread_mutex_lock(&g_input_mutex);
        local_input = g_input_state;
        g_input_state.fire = false;  /* Reset fire for next frame */
        if (g_input_state.quit) {
            game_state.running = false;
        }
        pthread_mutex_unlock(&g_input_mutex);
        
        /* ===== Update game state (no locks needed) ===== */
        update_player(&game_state, &local_input);
        update_enemies(&game_state);
        update_missiles(&game_state);
        spawn_enemy(&game_state);
        check_collisions(&game_state);
        
        /* ===== Render ===== */
        render_game(&game_state);
        screen_buffer_output();
        
        /* ===== FPS Control ===== */
        double frame_end_time = get_time_seconds();
        double frame_duration = (frame_end_time - current_time) * 1000.0;  /* in ms */
        double sleep_time = FRAME_TIME_MS - frame_duration;
        
        if (sleep_time > 1.0) {
            Sleep((DWORD)sleep_time);
        }
    }
}

/* ===== Main ===== */
int main(void)
{
    srand((unsigned)time(NULL));
    
    console_init();
    
    /* Start input thread */
    pthread_t input_tid;
    if (pthread_create(&input_tid, NULL, input_thread_func, NULL) != 0) {
        fprintf(stderr, "Failed to create input thread\n");
        console_cleanup();
        return 1;
    }
    
    /* Run game loop */
    game_loop();
    
    /* Wait for input thread to finish */
    pthread_join(input_tid, NULL);
    
    console_cleanup();
    
    printf("\nGame Over!\n");
    printf("Final Score: %d\n", 0);  /* Score could be tracked globally */
    
    return 0;
}

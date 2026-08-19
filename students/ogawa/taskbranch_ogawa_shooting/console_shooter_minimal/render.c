#include "render.h"

#include <stdio.h>
#include <string.h>

static char screen[SCREEN_HEIGHT][SCREEN_WIDTH];
static HANDLE output_handle = INVALID_HANDLE_VALUE;
static DWORD original_output_mode;
static int saved_output_mode;

static void put_char(int x, int y, char value)
{
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        screen[y][x] = value;
    }
}

void console_init(void)
{
    DWORD mode;

    output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (output_handle != INVALID_HANDLE_VALUE && GetConsoleMode(output_handle, &mode)) {
        original_output_mode = mode;
        saved_output_mode = 1;
        SetConsoleMode(output_handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }

    /* ANSI はカーソル制御とカーソル表示だけに使い、毎フレームの全消去はしない。 */
    fputs("\x1b[2J\x1b[H\x1b[?25l", stdout);
    fflush(stdout);
}

void console_cleanup(void)
{
    fputs("\x1b[?25h\x1b[H", stdout);
    fflush(stdout);

    if (saved_output_mode) {
        SetConsoleMode(output_handle, original_output_mode);
    }
}

void render_game(const GameState *game)
{
    char output[SCREEN_HEIGHT * (SCREEN_WIDTH + 2) + 64];
    char ui[SCREEN_WIDTH + 1];
    size_t output_length = 0;
    int x;
    int y;
    int index;

    /* 画面全体をメモリ上で再構築してから、一括で端末へ送る。 */
    memset(screen, ' ', sizeof(screen));
    snprintf(ui, sizeof(ui), "SCORE: %06d  A/D or arrows: MOVE  SPACE: FIRE  Q/ESC: QUIT", game->score);
    for (x = 0; ui[x] != '\0' && x < SCREEN_WIDTH; ++x) screen[0][x] = ui[x];

    if (game->player.active) {
        put_char(game->player.x, game->player.y - 2, '^');
        put_char(game->player.x - 1, game->player.y - 1, '/');
        put_char(game->player.x, game->player.y - 1, '|');
        put_char(game->player.x + 1, game->player.y - 1, '\\');
        put_char(game->player.x - 1, game->player.y, '/');
        put_char(game->player.x, game->player.y, '|');
        put_char(game->player.x + 1, game->player.y, '\\');
    }

    for (index = 0; index < MAX_ENEMIES; ++index) {
        if (game->enemies[index].active) {
            put_char((int)(game->enemies[index].x + 0.5),
                     (int)(game->enemies[index].y + 0.5), 'W');
        }
    }

    for (index = 0; index < MAX_MISSILES; ++index) {
        if (game->missiles[index].active) {
            put_char((int)(game->missiles[index].x + 0.5),
                     (int)(game->missiles[index].y + 0.5), '|');
        }
    }

    output[output_length++] = '\x1b';
    output[output_length++] = '[';
    output[output_length++] = 'H';
    for (y = 0; y < SCREEN_HEIGHT; ++y) {
        memcpy(output + output_length, screen[y], SCREEN_WIDTH);
        output_length += SCREEN_WIDTH;
        output[output_length++] = '\r';
        output[output_length++] = '\n';
    }
    fwrite(output, 1, output_length, stdout);
    fflush(stdout);
}
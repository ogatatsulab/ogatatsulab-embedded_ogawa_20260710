#include "render.h"
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* グローバル変数：コンソールハンドルと元のモード保存 */
static HANDLE console_handle = NULL;
static DWORD original_mode = 0;

bool render_init_console(void)
{
    /* コンソールハンドル取得 */
    console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (console_handle == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    /* 現在のモードを取得 */
    if (!GetConsoleMode(console_handle, &original_mode)) {
        return false;
    }
    
    /* ANSI/VTシーケンス有効化 */
    DWORD new_mode = original_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(console_handle, new_mode)) {
        return false;
    }
    
    /* カーソル非表示 */
    printf("\033[?25l");
    
    /* 画面クリア */
    printf("\033[2J");
    
    /* カーソルをホームポジションへ */
    printf("\033[H");
    
    fflush(stdout);
    
    return true;
}

void render_cleanup_console(void)
{
    if (console_handle == NULL) return;
    
    /* カーソル再表示 */
    printf("\033[?25h");
    
    /* 画面クリア */
    printf("\033[2J");
    
    /* カーソルをホームポジションへ */
    printf("\033[H");
    
    fflush(stdout);
    
    /* 元のコンソールモードに復元 */
    if (original_mode != 0) {
        SetConsoleMode(console_handle, original_mode);
    }
}

void render_clear_buffer(char buffer[SCREEN_HEIGHT][SCREEN_WIDTH])
{
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            buffer[y][x] = ' ';
        }
    }
}

void render_player(char buffer[SCREEN_HEIGHT][SCREEN_WIDTH], const Player *player)
{
    if (!player->active) return;
    
    int x = (int)player->x;
    int y = (int)player->y;
    
    /* 画面内かチェック */
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        buffer[y][x] = player->symbol;
    }
}

void render_enemies(char buffer[SCREEN_HEIGHT][SCREEN_WIDTH], const Enemy enemies[MAX_ENEMIES])
{
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) continue;
        
        int x = (int)enemies[i].x;
        int y = (int)enemies[i].y;
        
        if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
            buffer[y][x] = enemies[i].symbol;
        }
    }
}

void render_missiles(char buffer[SCREEN_HEIGHT][SCREEN_WIDTH], const Missile missiles[MAX_MISSILES])
{
    for (int i = 0; i < MAX_MISSILES; i++) {
        if (!missiles[i].active) continue;
        
        int x = (int)missiles[i].x;
        int y = (int)missiles[i].y;
        
        if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
            buffer[y][x] = missiles[i].symbol;
        }
    }
}

void render_ui(char buffer[SCREEN_HEIGHT][SCREEN_WIDTH], int score)
{
    /* スコアを画面右上に描画 */
    char score_str[20];
    snprintf(score_str, sizeof(score_str), "Score:%d", score);
    
    int x = SCREEN_WIDTH - (int)strlen(score_str);
    if (x < 0) x = 0;
    
    for (int i = 0; score_str[i] && x + i < SCREEN_WIDTH; i++) {
        buffer[0][x + i] = score_str[i];
    }
}

void render_present(const char buffer[SCREEN_HEIGHT][SCREEN_WIDTH])
{
    /* カーソルをホームポジションに移動 */
    printf("\033[H");
    
    /* 画面バッファを出力 */
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            putchar(buffer[y][x]);
        }
        putchar('\n');
    }
    
    fflush(stdout);
}

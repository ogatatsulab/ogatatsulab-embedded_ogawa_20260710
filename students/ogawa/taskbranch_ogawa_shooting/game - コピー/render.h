#ifndef RENDER_H
#define RENDER_H

#include "game.h"
#include <stdbool.h>

/* コンソール初期化と終了処理 */
bool render_init_console(void);
void render_cleanup_console(void);

/* 画面バッファ管理 */
void render_clear_buffer(char buffer[SCREEN_HEIGHT][SCREEN_WIDTH]);

/* オブジェクト描画 */
void render_player(char buffer[SCREEN_HEIGHT][SCREEN_WIDTH], const Player *player);
void render_enemies(char buffer[SCREEN_HEIGHT][SCREEN_WIDTH], const Enemy enemies[MAX_ENEMIES]);
void render_missiles(char buffer[SCREEN_HEIGHT][SCREEN_WIDTH], const Missile missiles[MAX_MISSILES]);

/* UI描画 */
void render_ui(char buffer[SCREEN_HEIGHT][SCREEN_WIDTH], int score);

/* コンソール出力 */
void render_present(const char buffer[SCREEN_HEIGHT][SCREEN_WIDTH]);

#endif /* RENDER_H */

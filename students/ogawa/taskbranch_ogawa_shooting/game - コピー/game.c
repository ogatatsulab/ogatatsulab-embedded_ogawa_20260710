#include "game.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

void game_init(GameState *state)
{
    memset(state, 0, sizeof(GameState));
    
    /* プレイヤー初期化 */
    state->player.x = SCREEN_WIDTH / 2.0f;
    state->player.y = SCREEN_HEIGHT - 3.0f;
    state->player.vx = 0.0f;
    state->player.vy = 0.0f;
    state->player.speed = PLAYER_SPEED;
    state->player.active = true;
    state->player.symbol = 'A';
    
    /* 敵初期化 */
    for (int i = 0; i < MAX_ENEMIES; i++) {
        state->enemies[i].active = false;
        state->enemies[i].symbol = 'V';
    }
    
    /* ミサイル初期化 */
    for (int i = 0; i < MAX_MISSILES; i++) {
        state->missiles[i].active = false;
        state->missiles[i].symbol = '|';
    }
    
    state->score = 0;
    state->enemy_count = 0;
    state->running = true;
    state->game_over = false;
    state->spawn_timer = 0.0f;
}

void update_player(GameState *state, float delta_time,
                   bool key_left, bool key_right,
                   bool key_up, bool key_down,
                   bool key_fire)
{
    if (!state->player.active) return;
    
    /* 速度リセット */
    state->player.vx = 0.0f;
    state->player.vy = 0.0f;
    
    /* 入力に基づいて速度設定 */
    if (key_left)  state->player.vx -= state->player.speed;
    if (key_right) state->player.vx += state->player.speed;
    if (key_up)    state->player.vy -= state->player.speed;
    if (key_down)  state->player.vy += state->player.speed;
    
    /* 位置更新 */
    state->player.x += state->player.vx * delta_time;
    state->player.y += state->player.vy * delta_time;
    
    /* 画面外判定：左右 */
    if (state->player.x < 0.0f) state->player.x = 0.0f;
    if (state->player.x >= SCREEN_WIDTH) state->player.x = SCREEN_WIDTH - 1.0f;
    
    /* 画面外判定：上下 */
    if (state->player.y < 0.0f) state->player.y = 0.0f;
    if (state->player.y >= SCREEN_HEIGHT) state->player.y = SCREEN_HEIGHT - 1.0f;
    
    /* ミサイル発射 */
    if (key_fire) {
        fire_missile(state);
    }
}

void update_enemies(GameState *state, float delta_time)
{
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!state->enemies[i].active) continue;
        
        /* 敵を下方向に移動 */
        state->enemies[i].y += ENEMY_SPEED * delta_time;
        
        /* 画面外に出たら非アクティブ化 */
        if (state->enemies[i].y >= SCREEN_HEIGHT) {
            state->enemies[i].active = false;
            state->enemy_count--;
        }
    }
}

void update_missiles(GameState *state, float delta_time)
{
    for (int i = 0; i < MAX_MISSILES; i++) {
        if (!state->missiles[i].active) continue;
        
        /* ミサイルを上方向に移動 */
        state->missiles[i].y += state->missiles[i].vy * delta_time;
        
        /* 画面外に出たら非アクティブ化 */
        if (state->missiles[i].y < 0.0f) {
            state->missiles[i].active = false;
        }
    }
}

void spawn_enemy(GameState *state)
{
    /* アクティブな敵が少ないなら新しい敵を生成 */
    if (state->enemy_count >= MAX_ENEMIES * 3 / 4) {
        return;
    }
    
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!state->enemies[i].active) {
            state->enemies[i].x = (float)(rand() % SCREEN_WIDTH);
            state->enemies[i].y = 0.0f;
            state->enemies[i].vx = 0.0f;
            state->enemies[i].vy = ENEMY_SPEED;
            state->enemies[i].active = true;
            state->enemy_count++;
            break;
        }
    }
}

void fire_missile(GameState *state)
{
    /* 非アクティブなミサイルを探して発射 */
    for (int i = 0; i < MAX_MISSILES; i++) {
        if (!state->missiles[i].active) {
            state->missiles[i].x = state->player.x;
            state->missiles[i].y = state->player.y - 1.0f;
            state->missiles[i].vx = 0.0f;
            state->missiles[i].vy = -MISSILE_SPEED;
            state->missiles[i].active = true;
            break;
        }
    }
}

void check_collisions(GameState *state)
{
    /* ミサイルと敵の衝突判定 */
    for (int i = 0; i < MAX_MISSILES; i++) {
        if (!state->missiles[i].active) continue;
        
        for (int j = 0; j < MAX_ENEMIES; j++) {
            if (!state->enemies[j].active) continue;
            
            /* 当たり判定：矩形判定（1x1） */
            int mx = (int)state->missiles[i].x;
            int my = (int)state->missiles[i].y;
            int ex = (int)state->enemies[j].x;
            int ey = (int)state->enemies[j].y;
            
            if (mx == ex && my == ey) {
                state->missiles[i].active = false;
                state->enemies[j].active = false;
                state->enemy_count--;
                state->score += 1;
            }
        }
    }
}

void game_update(GameState *state, float delta_time)
{
    if (!state->running) return;
    
    /* 敵生成タイマー */
    state->spawn_timer += delta_time;
    if (state->spawn_timer >= 0.5f) {  /* 0.5秒ごとに敵生成 */
        spawn_enemy(state);
        state->spawn_timer = 0.0f;
    }
}

#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

#define SCREEN_WIDTH  80
#define SCREEN_HEIGHT 24

#define MAX_ENEMIES   64
#define MAX_MISSILES  128

#define PLAYER_SPEED     40.0f  /* pixels per second */
#define MISSILE_SPEED    100.0f /* pixels per second */
#define ENEMY_SPEED      1.0f  /* pixels per second */

/* Player構造体 */
typedef struct {
    float x, y;          /* 位置 */
    float vx, vy;        /* 速度 */
    float speed;         /* 移動速度 */
    bool active;         /* アクティブフラグ */
    char symbol;         /* 描画文字 */
} Player;

/* Enemy構造体 */
typedef struct {
    float x, y;          /* 位置 */
    float vx, vy;        /* 速度 */
    bool active;         /* アクティブフラグ */
    char symbol;         /* 描画文字 */
} Enemy;

/* Missile構造体 */
typedef struct {
    float x, y;          /* 位置 */
    float vx, vy;        /* 速度 */
    bool active;         /* アクティブフラグ */
    char symbol;         /* 描画文字 */
} Missile;

/* ゲーム状態 */
typedef struct {
    Player player;
    Enemy enemies[MAX_ENEMIES];
    Missile missiles[MAX_MISSILES];
    
    int score;
    int enemy_count;
    
    bool running;
    bool game_over;
    
    float spawn_timer;  /* 敵生成タイマー */
} GameState;

/* ゲーム初期化 */
void game_init(GameState *state);

/* ゲーム更新 */
void game_update(GameState *state, float delta_time);

/* プレイヤー更新 */
void update_player(GameState *state, float delta_time,
                   bool key_left, bool key_right, 
                   bool key_up, bool key_down,
                   bool key_fire);

/* 敵更新 */
void update_enemies(GameState *state, float delta_time);

/* ミサイル更新 */
void update_missiles(GameState *state, float delta_time);

/* 敵生成 */
void spawn_enemy(GameState *state);

/* ミサイル発射 */
void fire_missile(GameState *state);

/* 衝突判定 */
void check_collisions(GameState *state);

#endif /* GAME_H */

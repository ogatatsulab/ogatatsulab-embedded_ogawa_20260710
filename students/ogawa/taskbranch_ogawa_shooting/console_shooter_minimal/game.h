#ifndef GAME_H
#define GAME_H

#include <windows.h>

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

#define FPS 20
#define FRAME_TIME_MS (1000 / FPS)

#define MAX_ENEMIES 20
#define MAX_MISSILES 50

/* 速度は画面セル毎秒。値を変更してゲーム速度を調整できる。 */
#define PLAYER_SPEED 24.0
#define ENEMY_SPEED 1.0
#define PLAYER_MISSILE_SPEED 10.0
#define ENEMY_SPAWN_INTERVAL_MS 2000

#define SCORE_PER_ENEMY 100

typedef struct {
    int x;
    int y;
    int active;
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
    Player player;
    Enemy enemies[MAX_ENEMIES];
    Missile missiles[MAX_MISSILES];
    int score;
    int running;
    int game_over;
    double spawn_elapsed_ms;
} GameState;

typedef struct {
    int left;
    int right;
    int fire;
    int quit;
} InputState;

void game_init(GameState *game);
void game_update(GameState *game, const InputState *input, double delta_seconds);
void update_player(GameState *game, const InputState *input, double delta_seconds);
void update_enemies(GameState *game, double delta_seconds);
void update_missiles(GameState *game, double delta_seconds);
void spawn_enemy(GameState *game);
void fire_missile(GameState *game);
void check_collisions(GameState *game);

#endif
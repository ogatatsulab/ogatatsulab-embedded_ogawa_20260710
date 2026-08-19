#include "game.h"

#include <stdlib.h>
#include <string.h>

static int clamp_int(int value, int minimum, int maximum)
{
    if (value < minimum) return minimum;
    if (value > maximum) return maximum;
    return value;
}

void game_init(GameState *game)
{
    memset(game, 0, sizeof(*game));
    game->player.x = SCREEN_WIDTH / 2;
    game->player.y = SCREEN_HEIGHT - 2;
    game->player.active = 1;
    game->running = 1;
}

void update_player(GameState *game, const InputState *input, double delta_seconds)
{
    int direction = input->right - input->left;
    int movement = (int)(direction * PLAYER_SPEED * delta_seconds);

    /* 座標を変更するのはゲームスレッドだけ。入力スレッドは座標に触れない。 */
    game->player.x = clamp_int(game->player.x + movement, 1, SCREEN_WIDTH - 2);
}

void spawn_enemy(GameState *game)
{
    int index;

    for (index = 0; index < MAX_ENEMIES; ++index) {
        if (!game->enemies[index].active) {
            game->enemies[index].x = 1 + rand() % (SCREEN_WIDTH - 2);
            game->enemies[index].y = 1;
            game->enemies[index].active = 1;
            return;
        }
    }
}

void update_enemies(GameState *game, double delta_seconds)
{
    int index;

    game->spawn_elapsed_ms += delta_seconds * 1000.0;
    if (game->spawn_elapsed_ms >= ENEMY_SPAWN_INTERVAL_MS) {
        game->spawn_elapsed_ms -= ENEMY_SPAWN_INTERVAL_MS;
        spawn_enemy(game);
    }

    for (index = 0; index < MAX_ENEMIES; ++index) {
        if (!game->enemies[index].active) continue;

        game->enemies[index].y += ENEMY_SPEED * delta_seconds;
        if (game->enemies[index].y >= SCREEN_HEIGHT - 3) {
            game->enemies[index].active = 0;
            game->running = 0;
            game->game_over = 1;
        }
    }
}

void fire_missile(GameState *game)
{
    int index;

    for (index = 0; index < MAX_MISSILES; ++index) {
        if (!game->missiles[index].active) {
            game->missiles[index].x = game->player.x;
            game->missiles[index].y = game->player.y - 2;
            game->missiles[index].active = 1;
            return;
        }
    }
}

void update_missiles(GameState *game, double delta_seconds)
{
    int index;

    for (index = 0; index < MAX_MISSILES; ++index) {
        if (!game->missiles[index].active) continue;

        game->missiles[index].y -= PLAYER_MISSILE_SPEED * delta_seconds;
        if (game->missiles[index].y < 1) {
            game->missiles[index].active = 0;
        }
    }
}

void check_collisions(GameState *game)
{
    int missile_index;
    int enemy_index;

    for (missile_index = 0; missile_index < MAX_MISSILES; ++missile_index) {
        if (!game->missiles[missile_index].active) continue;

        for (enemy_index = 0; enemy_index < MAX_ENEMIES; ++enemy_index) {
            if (!game->enemies[enemy_index].active) continue;

            if ((int)(game->missiles[missile_index].x + 0.5) ==
                    (int)(game->enemies[enemy_index].x + 0.5) &&
                (int)(game->missiles[missile_index].y + 0.5) ==
                    (int)(game->enemies[enemy_index].y + 0.5)) {
                game->missiles[missile_index].active = 0;
                game->enemies[enemy_index].active = 0;
                game->score += SCORE_PER_ENEMY;
                break;
            }
        }
    }
}

void game_update(GameState *game, const InputState *input, double delta_seconds)
{
    if (input->quit) {
        game->running = 0;
        return;
    }

    update_player(game, input, delta_seconds);
    if (input->fire) fire_missile(game);
    update_enemies(game, delta_seconds);
    update_missiles(game, delta_seconds);
    check_collisions(game);
}
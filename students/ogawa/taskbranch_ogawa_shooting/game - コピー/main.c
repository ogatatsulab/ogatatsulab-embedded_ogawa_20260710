#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"
#include "input.h"
#include "render.h"
#include "timer.h"

int main(void)
{
    /* コンソール初期化 */
    if (!render_init_console()) {
        fprintf(stderr, "コンソール初期化に失敗しました\n");
        return 1;
    }
    
    /* タイマー初期化（60 FPS） */
    Timer timer;
    if (!timer_init(&timer, 60.0f)) {
        fprintf(stderr, "タイマー初期化に失敗しました\n");
        render_cleanup_console();
        return 1;
    }
    
    /* ゲーム状態初期化 */
    GameState game_state;
    game_init(&game_state);
    
    /* 入力状態初期化 */
    InputState input_state;
    input_init(&input_state);
    
    /* Input Thread起動 */
    if (!input_start_thread(&input_state)) {
        fprintf(stderr, "Input Thread起動に失敗しました\n");
        render_cleanup_console();
        return 1;
    }
    
    /* 画面バッファ */
    char screen_buffer[SCREEN_HEIGHT][SCREEN_WIDTH];
    
    /* メインゲームループ */
    while (game_state.running) {
        /* 1. delta time計算 */
        timer_frame_start(&timer);
        float delta_time = timer_get_delta_time(&timer);
        
        /* 2. 入力状態読み込み */
        bool key_left, key_right, key_up, key_down, key_fire, key_escape;
        input_read_state(&input_state, &key_left, &key_right, &key_up, &key_down, &key_fire, &key_escape);
        
        /* ESCで終了 */
        if (key_escape) {
            game_state.running = false;
            break;
        }
        
        /* 3. ゲーム状態更新 */
        game_update(&game_state, delta_time);
        update_player(&game_state, delta_time, key_left, key_right, key_up, key_down, key_fire);
        update_enemies(&game_state, delta_time);
        update_missiles(&game_state, delta_time);
        
        /* 4. 衝突判定 */
        check_collisions(&game_state);
        
        /* 5. 画面バッファをクリア */
        render_clear_buffer(screen_buffer);
        
        /* 6. オブジェクト描画 */
        render_player(screen_buffer, &game_state.player);
        render_enemies(screen_buffer, game_state.enemies);
        render_missiles(screen_buffer, game_state.missiles);
        
        /* 7. UI描画 */
        render_ui(screen_buffer, game_state.score);
        
        /* 8. コンソール出力 */
        render_present(screen_buffer);
        
        /* 9. FPS制御（必要な時間待機） */
        timer_frame_end(&timer);
    }
    
    /* クリーンアップ */
    input_stop_thread(&input_state);
    render_cleanup_console();
    
    printf("ゲーム終了\n最終スコア: %d\n", game_state.score);
    
    return 0;
}

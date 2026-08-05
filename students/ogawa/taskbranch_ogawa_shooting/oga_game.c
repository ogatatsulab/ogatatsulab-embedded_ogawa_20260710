/*
 ShootingGameSample by T.Ogawa(2025/03/17)
   第2版: 2025/03/19
*/



#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>



// Mutex
pthread_mutex_t lock_mutex = PTHREAD_MUTEX_INITIALIZER;



// ゲーム画面サイズ
#define LINE 15    // ゲーム画面行数
#define COL 26     // ゲーム画面桁数



// 敵数
#define ENEMY_COUNT 4



// プレーヤー型
typedef struct {
  int line;          // 行位置
  int col;           // 桁位置
  int move_right;    // 右方向移動指示（dが押されたら1）
  int move_left;     // 左方向移動指示（aが押されたら1）
  char symbol;       // キャラクタ
} Player;

// 敵型
typedef struct {
  int line;         // 行位置
  int col;          // 桁位置
  int direction;    // 移動方向（1:右 2:左）
  int hit;          // ヒット（0:未ヒット 1:ヒット済み）
  char symbol;      // キャラクタ
} Enemy;

// ミサイル型
typedef struct {
  int shoot;    // 飛行中フラグ（画面に表示されている間1）
  int line;     // 行位置
  int col;      // 桁位置
  char symbol;  // キャラクタ
} Missile;



// 画面（この内容を一定間隔で画面に表示する）
char gamen[ LINE + 1 ][ COL + 3 ];

// 初期画面（枠組みだけ。これにプレーヤーと敵とミサイルを書く）
char gamen_template[ LINE + 1 ][ COL + 3 ];



// プレーヤー
Player player = { LINE -1 , COL / 2 + 1, 0, 0, 'M' };    // 最下部中央

// 敵
Enemy enemy[ ENEMY_COUNT ];
int hit_count = 0;   // 当たりカウント

// ミサイル
Missile missile = { 0, 0, 0, '|' };    // 画面に表示しない 



// フラグ
int flg_end = 0;    // ゲーム終了フラグ（1:プレーヤー勝ち 2:プレーヤー負け）
int flg_shoot = 0;  // ミサイル発射指示フラグ（スペースが押されたら1）



// 関数プロトタイプ
int main( void );
void init_gamen( void );
void print_gamen( void );
void* game_main( void* p );
void move_enemy( void );
void move_enemy2( int i );
void move_player( void );
void move_missile( void );
void judge_hit( void );
void* key_hit( void* p );
void init_enemy( void );
void print_special( char* line, char special );



// main
int main( void ) {
	
	printf( "%d\n", player.line );
  init_enemy();          // 敵準備
  init_gamen();          // 画面準備
    
  pthread_t thread1, thread2;

  pthread_create( &thread1, NULL, game_main, NULL );
  pthread_create( &thread2, NULL, key_hit, NULL );

  pthread_join( thread1, NULL );
  pthread_join( thread2, NULL );

  pthread_mutex_destroy(&lock_mutex);    // Mutex削除
  
  printf( "\e[?25h" );   // カーソル表示

  return 0;
}



// 画面準備
void init_gamen( void ) {
  system( "cls" );
  printf( "\e[?25l" );                                 // カーソル非表示
  memset( gamen, ' ', sizeof( gamen ) );               // すべてスペース埋め
  for ( int i = 0; i < ( LINE + 1 ); i++ ) {           // 左右枠と行末のヌル文字
    gamen[ i ][ 0 ] = '|';
    gamen[ i ][ COL + 1 ] = '|';
    gamen[ i ][ COL + 2 ] = '\0';
  }
  for ( int i = 1; i < ( COL + 1 ); i++ ) {            // 下枠
    gamen[ LINE ][ i ] = '-';
  }
  gamen[ LINE ][ 0 ] = '+';                            // 角
  gamen[ LINE ][ COL + 1 ] = '+';
  memcpy( gamen_template, gamen, sizeof( gamen ) );    // これを初期画面として保存する
  return;
}



// 敵準備
void init_enemy( void ) {
  srand( (unsigned)time( NULL ) );
  for ( int i = 0; i < ENEMY_COUNT; i++ ) {    // 上から敵数行の間のランダム位置に敵を配置
    enemy[ i ].col = rand() % COL + 1;
    enemy[ i ].line = rand() % ENEMY_COUNT + 0;
    enemy[ i ].direction = rand() % 2 + 1;
    enemy[ i ].hit = 0;
    if ( ( rand() % 10 + 0 ) == 0 ) {          // 10%の確率でスペシャル敵が出現
      enemy[ i ].symbol = '$';
    } else {
      enemy[ i ].symbol = '@';
    }
  }
  return;
}



// 初期画面に戻す
void clear_gamen( void ) {
  memcpy( gamen, gamen_template, sizeof( gamen ) );    // 初期画面に戻す（枠以外はすべて空白）
  return;
}



// 画面表示
void print_gamen( void ) {
  printf( "\e[1;1H" );
  for ( int i = 0; i < ( LINE + 1 ); i++ ) {                   // 横一列一行ずつ画面出力
//    printf( "%s\n", gamen[ i ] );
    print_special( gamen[ i ], '$' );                          // '$'を赤色にする（特別キャラとしてスコアアップなどの要改良）
  }
  printf( "\n左移動:a 右移動:d ミサイル発射:space\n\n" );
  printf( "Hit(Total): %d(%d)\n", hit_count, ENEMY_COUNT );    // ヒット状態表示


  return;
}



// ゲームループ処理
void* game_main( void* p ) {
  while ( flg_end  == 0 ) {    // flg_endが0の間ループ（弱点：プレーヤー、敵、ミサイルなどのスピード感の細かいコントロールなどが微妙）
    usleep( 80000 );
    clear_gamen();             // 画面初期化
    move_enemy();              // 敵セット
    move_player();             // プレーヤーセット
    move_missile();            // ミサイルセット
    print_gamen();             // 画面表示
    judge_hit();               // 当たり判定
  }
  system( "cls" );             // 結果表示
  if ( flg_end == 1 ) {
    printf( "\n*** YOU WIN! ***\n" );
  } else if ( flg_end == 2 ) {
    printf( "\n*** YOU LOSE! ***\n" );
  }
  return NULL;
}



// 敵セット
void move_enemy( void ) {
  for ( int i = 0; i < ENEMY_COUNT; i++ ) {
    if ( enemy[ i ].hit == 0 ) {
      move_enemy2( i );
    }
  }
  return;
}



void move_enemy2( int i ) {
  if ( enemy[ i ].direction == 1 ) {               // 右方向に移動中
    if ( enemy[ i ].col == COL ) {                 // 右端なら
      if ( ++enemy[ i ].line > ( LINE - 2 ) ) {    // 一段下へ（最下部ならゲームセット、プレーヤー負け）
        flg_end = 2;
      } else {
        enemy[ i ].direction = 2;                  // 移動方向を左へ
      }
    } else {
      enemy[ i ].col++;                            // 右へ移動
    }
  } else if ( enemy[ i ].direction == 2 ) {        // 左方向に移動中
    if ( enemy[ i ].col == 1 ) {                   // 左端なら
      if ( ++enemy[ i ].line > ( LINE - 2 ) ) {    // 一段下へ（最下部ならゲームセット、プレーヤー負け）
        flg_end = 2;
      } else {
        enemy[ i ].direction = 1;                  // 移動方向を右へ
      }
    } else {
      enemy[ i ].col--;                            // 左へ移動
    }
  }
  gamen[ enemy[ i ].line ][ enemy[ i ].col ] = enemy[ i ].symbol;
  return;
}



// プレーヤーセット
void move_player( void ) {
  pthread_mutex_lock( &lock_mutex );
  if ( player.move_left == 1 ) {            // 左移動指示
    if ( player.col > 1 ) {                 // 左端でなかったら左へ移動
      player.col--;
    }
    player.move_left = 0;
  } else if ( player.move_right == 1 ) {    // 右移動指示
    if ( player.col < COL ) {               // 右端でなかったら右へ移動
      player.col++;
    }
    player.move_right = 0;
  }
  pthread_mutex_unlock( &lock_mutex );
  gamen[ player.line ][ player.col ] = player.symbol;
  return;
}



// ミサイルセット
void move_missile( void ) {
  pthread_mutex_lock( &lock_mutex );
  int shoot = flg_shoot;
  flg_shoot = 0;
  pthread_mutex_unlock( &lock_mutex );
	
  if ( shoot == 1 && missile.shoot == 0 ) {                 // 飛行中でない時に発射指示があったら
    missile.shoot = 1;                                          // 飛行中にする
    missile.col = player.col;                                   // 初期位置にセット
    missile.line = player.line - 1;
    gamen[ missile.line ][ missile.col ] = missile.symbol;
  } else if ( missile.shoot == 1 ) {
    if ( --missile.line == 0 ) {                                // 最上部なら飛行終了（表示されなくなる）
      missile.shoot = 0;
    } else {
      gamen[ missile.line ][ missile.col ] = missile.symbol;    // それ以外なら上へ移動
    }
  }
  return;
}



// 当たり判定
void judge_hit( void ) {
  for ( int i = 0; i < ENEMY_COUNT; i++ ) {
    if ( ( enemy[ i ].hit == 0 ) && ( enemy[ i ].col == missile.col ) && ( enemy[ i ].line == missile.line ) ) {    // 敵とミサイルが同位置ならヒット
      if( ( ++hit_count >= ENEMY_COUNT ) || ( enemy[ i ].symbol == '$' ) ) {                                        // 全敵ヒットかスペシャル敵ならゲームセット（プレーヤー勝ち）
        flg_end = 1;
      }
      enemy[ i ].hit = 1;
    }
  }
  return;
}



// 押下キー取得
void* key_hit( void* p ) {
  while( flg_end == 0 )  {          // flg_endが0の間ループ
    if ( kbhit() ) {
      char key = getch();
      switch ( key ) {
        case ' ':
          pthread_mutex_lock( &lock_mutex );
          flg_shoot = 1;            // ミサイル発射指示
          pthread_mutex_unlock( &lock_mutex );
          break;
        case 'a':
          pthread_mutex_lock( &lock_mutex );
          player.move_left = 1;     // プレーヤー左移動指示
          pthread_mutex_unlock( &lock_mutex );
          break;
        case 'd':
          pthread_mutex_lock( &lock_mutex );
          player.move_right = 1;    // プレーヤー右移動指示
          pthread_mutex_unlock( &lock_mutex );
          break;
      }
    }
    usleep( 50000 );
  }
  return NULL;
}



// 文字列内の特定文字を赤色にする
void print_special( char* line, char special ) {
  char* idx = strchr( line, special );
  while ( idx != NULL ) {
    int idx2 = idx - line;
    char buf[ strlen( line ) ];
    strcpy( buf, line );
    buf[ idx2 ] = '\0';
    printf( "%s", buf );

    // ここで文字色指定
    printf( "\e[31m" );

    printf( "\e[31m%c\e[39m", line[ idx2 ] );
    line += idx2 + 1;
    idx = strchr( line, special );
  }
  printf( "%s\n", line );
  return;
}

#include <stdio.h>
#include <unistd.h>
#include <windows.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

#define NUM_OBJECTS 5
#define MAX_COL 80
#define MAX_ROW 24

typedef struct {
	int row;
	int col;
	int col_prev;
} Object;

int main( void );
void* move( void* p );

void* move( void* p ) {
	Object* obj = (Object*)p;
	
	while ( 1 ) {
		// 前の位置を消す
		printf( "\e[%d;%dH ", obj->row, obj->col_prev );
		
		// 新しい位置に移動
		obj->col_prev = obj->col;
		obj->col++;
		
		if ( obj->col > MAX_COL ) {
			obj->col = 1;
		}
		
		// 新しい位置に描画
		printf( "\e[%d;%dH@", obj->row, obj->col );
		fflush( stdout );
		
		Sleep( 100 );
	}
	
	return NULL;
}

int main( void ) {
	
	system( "cls" );	// 実行必須
	printf( "\e[?25l" );  // カーソル非表示
	
	srand( (unsigned int)time( NULL ) );
	
	Object objects[NUM_OBJECTS];
	pthread_t threads[NUM_OBJECTS];
	
	// オブジェクトを初期化してスレッド作成
	for ( int i = 0; i < NUM_OBJECTS; i++ ) {
		objects[i].row = (rand() % MAX_ROW) + 1;
		objects[i].col = (rand() % MAX_COL) + 1;
		objects[i].col_prev = objects[i].col;
		
		pthread_create( &threads[i], NULL, move, &objects[i] );
	}
	
	// スレッドの終了を待つ
	for ( int i = 0; i < NUM_OBJECTS; i++ ) {
		pthread_join( threads[i], NULL );
	}
	
	printf( "\e[?25h" );  // カーソル表示（実行しないとコマンドプロンプトのカーソルがない
	
	return 0;
}


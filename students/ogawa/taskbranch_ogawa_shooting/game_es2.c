#include <stdio.h>
#include <unistd.h>
#include <windows.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

int main( void );
void* move( void* p );

#define STAR_COUNT 8
#define MAX_ROW 20
#define MAX_COL 30

typedef struct {
	int row;
	int col;
	unsigned int seed;
} Star;

pthread_mutex_t screen_mutex = PTHREAD_MUTEX_INITIALIZER;

static unsigned int random_value( unsigned int* seed ) {
	*seed = *seed * 1103515245u + 12345u;
	return ( *seed / 65536u ) % 32768u;
}

void* move( void* p ) {
	Star* star = (Star*)p;

	while ( 1 ) {
		int old_row = star->row;
		int old_col = star->col;
		int new_row = old_row;
		int new_col = old_col;

		if ( random_value( &star->seed ) % 2 == 0 ) {
			new_row += (int)( random_value( &star->seed ) % 3 ) - 1;
		} else {
			new_col += (int)( random_value( &star->seed ) % 3 ) - 1;
		}

		if ( new_row < 1 ) new_row = 1;
		if ( new_row > MAX_ROW ) new_row = MAX_ROW;
		if ( new_col < 1 ) new_col = 1;
		if ( new_col > MAX_COL ) new_col = MAX_COL;

		pthread_mutex_lock( &screen_mutex );
		printf( "\e[%d;%dH ", old_row, old_col );
		star->row = new_row;
		star->col = new_col;
		printf( "\e[%d;%dH@", new_row, new_col );
		fflush( stdout );
		pthread_mutex_unlock( &screen_mutex );

		Sleep( 100 );
	}

	return NULL;
}

int main( void ) {
	pthread_t threads[STAR_COUNT];
	Star stars[STAR_COUNT];

	system( "cls" );	// 実行必須
	printf( "\e[?25l" );  // カーソル非表示

	for ( int i = 0; i < STAR_COUNT; i++ ) {
		stars[i].row = 1 + ( i * 3 ) % MAX_ROW;
		stars[i].col = 1 + ( i * 7 ) % MAX_COL;
		stars[i].seed = (unsigned int)time( NULL ) + (unsigned int)( i * 101 + 1 );
		printf( "\e[%d;%dH@", stars[i].row, stars[i].col );
		pthread_create( &threads[i], NULL, move, &stars[i] );
	}
	fflush( stdout );

	for ( int i = 0; i < STAR_COUNT; i++ ) {
		pthread_join( threads[i], NULL );
	}
	
	return 0;
}


#include <stdio.h>
#include <unistd.h>
#include <windows.h>
#include <pthread.h>

int main( void );
void* move( void* p );

int main( void ) {
	
	system( "cls" );	// 実行必須
	printf( "\e[?25l" );  // カーソル非表示
	
	while ( 1 ) {
		printf( "\e[1;30H " );
		for ( int i = 1; i <= 30; i++ ) {
			if ( i >= 2 ) {
				printf( "\e[1;%dH ", i - 1 );
			}
			printf( "\e[1;%dH@", i );
			Sleep( 100 );
		}
	}
	
	printf( "\e[?25h" );  // カーソル表示（実行しないとコマンドプロンプトのカーソルがない
	
	return 0;
}


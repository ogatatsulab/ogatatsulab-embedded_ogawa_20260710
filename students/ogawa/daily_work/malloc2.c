#include <stdio.h>
#include <stdlib.h>

int main( void ) {

  char *str;
  int length;

  printf( "何文字入力しますか?\n" );
  scanf( "%d", &length );
  getchar();
  
  str = malloc( ( length + 1 ) * sizeof( char ) );
  if ( str == NULL ) {
    printf( "メモリの確保に失敗しました\n" );
    return 1;
  }

  printf( "それでは%d文字入力してください\n", length );
  fgets( str, length + 1, stdin );
  printf( "入力値: %s\n", str );

  free( str );  // 大事！！！

  return 0;

}
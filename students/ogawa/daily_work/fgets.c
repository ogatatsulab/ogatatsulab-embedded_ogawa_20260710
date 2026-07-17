#include <stdio.h>
#include <string.h>

int count = 0;

int main( void ) {

  char string[ 5 ];

  while ( 1 ) {     //1

    printf( "Input String => " );
    fgets( string, sizeof( string ), stdin );

    printf( "string(%d): %s\n", ++count, string );

    if ( ! strchr( string, '\n' ) ) {    //3
      while( getchar() != '\n' );     //2
    }     //3

  }     //1

  return 0;

}

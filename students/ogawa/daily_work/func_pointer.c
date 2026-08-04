#include <stdio.h>

int main( void );
int add( int a, int b );
int subtract( int a, int b );

int main( void ) {

  printf( "add: %d\n", add( 5, 3 ) );
  printf( "sub: %d\n", subtract( 5, 3 ) );

  return 0;

}

int add( int a, int b ) {
  return a + b;
}

int subtract( int a, int b ) {
  return a - b;
}

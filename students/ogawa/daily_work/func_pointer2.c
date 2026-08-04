#include <stdio.h>

int main( void );
int add( int a, int b );
int subtract( int a, int b );

int main( void ) {

/*
  printf( "add: %d\n", add( 5, 3 ) );
  printf( "sub: %d\n", subtract( 5, 3 ) );
*/

  int (*func_p)(int,int); // ŠÖ”ƒ|ƒCƒ“ƒ^

  func_p = &add;
  printf( "add: %d\n", func_p( 5, 3 ) );

  func_p = subtract;  // &‚Í‚È‚­‚Ä‚à‚¢‚¢
  printf( "sub: %d\n", func_p( 5, 3 ) );

  return 0;

}

int add( int a, int b ) {
  return a + b;
}

int subtract( int a, int b ) {
  return a - b;
}

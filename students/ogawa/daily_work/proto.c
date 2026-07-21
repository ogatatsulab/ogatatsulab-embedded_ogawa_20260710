#include <stdio.h>

// Function Prototype
int main( void );
int tasu( int a, int b );
int kakeru( int a, int b );
void msg( void );


int main( void ) {

  int a = 2;
  int b = 5;

  printf( "%d + %d = %d\n", a, b, tasu( a, b ) );
  printf( "%d x %d = %d\n", a, b, kakeru( a, b ) );

  msg();

  return 123;

}


int tasu( int a, int b ) {

  return ( a + b );

}



int kakeru( int a, int b ) {

  return ( a * b );

}


void msg( void ) {
  printf( "Message!!\n" );
  return;
}
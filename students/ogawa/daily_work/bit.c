#include <stdio.h>
#include <limits.h>



int main( void );
void print_bits( unsigned int x );



/*
x |=  1U << n;        // n番目を1にする
x &= ~( 1U << n );     // n番目を0にする
x ^=  1U << n;       // n番目を反転する
( x >> n ) & 1U;       // n番目の値を取り出す
*/



int main( void ) {

  unsigned int x = 3;
  puts( "\n元データ" );
  print_bits( x );

  puts( "\n5番目を1にする" );
  x |=  1U << 5;       // n番目を1にする
  print_bits( x );

  puts( "\n0番目を0にする" );
  x &= ~( 1U << 0 );     // n番目を0にする
  print_bits( x );

  puts( "\n4番目を反転する" );
  x ^=  1U << 4;       // n番目を反転する
  print_bits( x );

  puts( "\n5番目の値を取り出す" );
  printf( "%u\n", ( x >> 5 ) & 1U );       // n番目の値を取り出す

  return 0;

}



void print_bits( unsigned int x ) {

    int bit_count = sizeof( x ) * CHAR_BIT; 
    // printf( "sizeof(unsined int): %d\n", sizeof( unsigned int ) );
    // printf( "CHAR_BIT: %d\n", CHAR_BIT );
    // printf( "bit_count: %d\n", bit_count );

    for ( int i = bit_count - 1; i >= 0; i-- ) {
        unsigned int bit = ( x >> i ) & 1U;
        putchar( bit ? '1' : '0' );
    }
    putchar( '\n' );

}

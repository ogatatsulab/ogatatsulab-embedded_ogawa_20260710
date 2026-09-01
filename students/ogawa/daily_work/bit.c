/*====

&(論理積)
0 & 0 = 0
1 & 0 = 0
0 & 1 = 0
1 & 1 = 1

|(論理和)
0 | 0 = 0
1 | 0 = 1
0 | 1 = 1
1 | 1 = 1

^(排他的論理和)
0 ^ 0 = 0
1 ^ 0 = 1
0 ^ 1 = 1
1 ^ 1 = 0

~(ビット反転)
~01100110 = 10011001

====*/



#include <stdio.h>
#include <limits.h>



int main( void );
void print_bits( unsigned int x );




/*====

x |=  1U << n;		// n番目を1にする
x &= ~( 1U << n );	// n番目を0にする
x ^=  1U << n;		// n番目を反転する
( x >> n ) & 1U;	// n番目の値を取り出す

====*/



int main( void ) {

  unsigned int x = 3;
  puts( "\n元データ" );
  print_bits( x );

/*====
  00000011
====*/



  puts( "\n5番目を1にする" );
  x |=  1U << 5;       // 5番目を1にする
  print_bits( x );

/*====
  x |= 1U << 5;
  00000011 |= 00000001 << 5;
  00000011 |= 00100000;
  00000011 | 00100000 = 00100011
====*/



  puts( "\n0番目を0にする" );
  x &= ~( 1U << 0 );     // 0番目を0にする
  print_bits( x );

/*====
  x &= ~( 1U << 0 );
  00100011 &= ~( 00000001 << 0 );
  00100011 &= ~( 00000001 );
  00100011 &= 11111110;
  00100011 & 11111110 = 00100010
====*/



  puts( "\n4番目を反転する" );
  x ^=  1U << 4;       // 4番目を反転する
  print_bits( x );

/*====
  x ^=  1U << 4;
  00100010 ^= 00000001 << 4;
  00100010 ^= 00010000;
  00100010 ^ 00010000 = 00110010
====*/



  puts( "\n5番目の値を取り出す" );
  printf( "%u\n", ( x >> 5 ) & 1U );       // n番目の値を取り出す

/*====
  ( x >> 5 ) & 1U
  ( 00110010 >> 5 ) & 00000001
  00000001 & 00000001 = 1
====*/



  return 0;

}



void print_bits( unsigned int x ) {

    int bit_count = sizeof( x ) * CHAR_BIT; 
//    printf( "sizeof(unsigned int): %lu\n", sizeof( unsigned int ) );
//    printf( "CHAR_BIT: %d\n", CHAR_BIT );
//    printf( "bit_count: %d\n", bit_count );

    for ( int i = bit_count - 1; i >= 0; i-- ) {
        unsigned int bit = ( x >> i ) & 1U;
        putchar( bit ? '1' : '0' );
    }
    putchar( '\n' );

}

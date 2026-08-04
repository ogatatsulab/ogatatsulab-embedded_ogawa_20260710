#include <stdio.h>

enum Color {
  RED,   // 0
  GREEN, // 1
  BLUE   // 2 
};

int main( void ) {

  int input;

  printf( "好きな色は? 0:red, 1:green, 2:blue\n" );
  scanf( "%d", &input );
  enum Color favorite = input;  // キャストはなくてもよい

  switch ( favorite ) {
    case RED:
      printf( "You like red!\n" );
      break;
    case GREEN:
      printf( "You like green!\n" );
      break;
    case BLUE:
      printf( "You like blue!\n" );
      break;
    default:
      printf( "該当なし\n" );
      break;
  }

  return 0;
}

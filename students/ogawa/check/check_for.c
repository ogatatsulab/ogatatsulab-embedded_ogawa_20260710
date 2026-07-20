#include <stdio.h>

int main ( void ) {


  int students[ 3 ][ 4 ] = {
    { 1, 78, 92, 65 },
    { 2, 81, 89, 72 },
    { 3, 69, 85, 70 },
  };


// [ 0 ][ 1 ] + [ 0 ][ 2 ] + [ 0 ][ 3 ]
// [ 1 ][ 1 ] + [ 0 ][ 2 ] + [ 0 ][ 3 ]
// [ 2 ][ 1 ] + [ 0 ][ 2 ] + [ 0 ][ 3 ]
  for ( int i = 0; i < 3; i++ ) {
    double total = 0;
    for ( int j = 1; j < 4; j++ ) {
       total += students[ i ][ j ];
    }
    printf( "%d番さんの平均点は%.1f点\n", students[ i ][ 0 ], total / 3 );
	
  }


// [ 0 ][ 1 ] + [ 1 ][ 1 ] + [ 2 ][ 1 ]
// [ 0 ][ 2 ] + [ 1 ][ 2 ] + [ 2 ][ 2 ]
// [ 0 ][ 3 ] + [ 1 ][ 3 ] + [ 2 ][ 3 ]
  for ( int i = 1; i < 4; i++ ) {
    double total = 0;
    for ( int j = 0; j < 3; j++ ) {
       total += students[ j ][ i ];
    }
    printf( "第%d科目の平均点は%.1f点\n", i, total / 3 );
	
  }

  return 0;

}

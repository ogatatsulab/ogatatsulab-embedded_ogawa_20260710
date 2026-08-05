#include <stdio.h>

int main( void ) {

  typedef unsigned int uint;
  typedef char ogawa;
  typedef ogawa ogawa2;

  typedef struct {
    int age;
    char name[ 30 ];
  } Person;

  Person my_person;

  uint x = 100;
  ogawa y = 'A';
  ogawa2 z = 'Z';

  printf( "x=%d, y=%c, z=%c\n", x, y, z );

  return 0;

}
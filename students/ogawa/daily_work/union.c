#include <stdio.h>

union SensorValue {
  int intVal;
  char charVal;
  float floatVal;
};

int main( void ) {

  union SensorValue s;

  s.intVal = 100;
  printf( "s.intVal = %d\n", s.intVal );

  s.charVal = 'A';
  printf( "s.charVal = %c\n", s.charVal );

  s.floatVal = 3.14;
  printf( "s.floatVal = %.2f\n", s.floatVal );

  s.intVal = 123;

  printf( "====\n" );
  printf( "s.intVal = %d\n", s.intVal );
  printf( "s.charVal = %c\n", s.charVal );
  printf( ".floatVal = %.2f\n", s.floatVal );
  
  return 0;
}
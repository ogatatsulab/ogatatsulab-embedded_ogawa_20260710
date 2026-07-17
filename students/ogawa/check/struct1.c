#include <stdio.h>

struct Person {
  char name[ 50 ];
  int age;
};

void printPerson( struct Person* p ) {
  printf( "名前: %s\n", p->name );
  printf( "年齢: %d\n", p->age );
}

int main( void ) {
	struct Person person1 = { "Suzuki", 30 };
	
	// printPerson関数を使用して、person1の情報を表示させてください。なお、ポインタを使用すること。
  printPerson( &person1 );

	
  int students[ 5 ] = { 40, 50, 60, 70, 80 };
    
  // ポインタを使用して、配列studentsの2番要素（今回だと60）を表示させてください。
  printf( "配列studentsの2番要素は%d\n", *( students + 2 ) );
    
  return 0;
}

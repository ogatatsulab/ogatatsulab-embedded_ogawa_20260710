#include <stdio.h>

/*
struct Person {
    char name[50];
    int age;
};
*/

// 変数宣言で "struct Person" ではなく "Person" だけで定義できる
typedef struct {
  char name[ 50 ];
  int age;
} Person;



/*@関数のプロトタイプ宣言をおこなうこと*/
int main( void );
void print_num( int* num_pointer );
void print_array( int* array_numbers, int index );
void print_struct( Person person );
void print_pointer( Person* person );



int main( void ) {
    //１）ポインタの基本操作ができますか？
    int number = 50;
    int* num_pointer = &number;
    print_num( num_pointer );
    
    
    //２）ポインタを利用して配列を操作できますか？
    int array_numbers[5] = {10, 11, 12, 13, 14};
    print_array( array_numbers, 2 );
    
    
    //３）関数間で構造体をやりとりできますか？
    Person person1 = { "Ichiro", 30 };
    print_struct( person1 );
    
    
    //４）ポインタを利用して関数間で構造体をやりとりできますか？
    Person person2 = { "Jiro", 26 };
    print_pointer( &person2 );
    
    
    return 0;
}

void print_num( int* num_pointer ){
    printf( "number is %d\n", *num_pointer );
}

void print_array( int* array_numbers, int index ){
    printf( "index %d is %d\n", index, *(array_numbers + index ) );
}

void print_struct( Person person ) {
    printf("struct Name: %s\n", person.name );
    printf("struct Age: %d\n", person.age );
}

void print_pointer( Person* person ) {
    printf("pointer Name: %s\n", person->name );
    printf("pointer Age: %d\n", person->age );
}

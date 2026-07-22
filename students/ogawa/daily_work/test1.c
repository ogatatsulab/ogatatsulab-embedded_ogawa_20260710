#include <stdio.h>

int main( void ) {
char* string1 = "ABC";

char* string2 = "abc";

char* string3 = "XyZ";



printf( "%s(%p)\n", string1, string1 );

printf( "%s(%p)\n", string2, string2 );

printf( "%s(%p)\n", string3, string3 );





char* string_arr1[ 3 ];

string_arr1[ 0 ] = string1;

string_arr1[ 1 ] = string2;

string_arr1[ 2 ] = string3;



printf( "%p(%p);\n", string_arr1[ 0 ], &string_arr1[ 0 ] );

printf( "%p(%p);\n", string_arr1[ 1 ], &string_arr1[ 1 ] );

printf( "%p(%p);\n", string_arr1[ 2 ], &string_arr1[ 2 ] );



printf( "%p\n", *( string_arr1 + 0 ) );

printf( "%p\n", *( string_arr1+ 1 ) );

printf( "%p\n", *( string_arr1 + 2 ) );



for ( int i = 0; i < 3; i++ ) {			

	printf( "%s\n", *( string_arr1 + i ) );		

}			

			

			

char** string_arr2 = string_arr1;			

			

for ( int i = 0; i < 3; i++ ) {			

	printf( "%s\n", *( string_arr2 + i ) );		

}			

}

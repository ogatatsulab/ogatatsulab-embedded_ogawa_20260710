#include <stdio.h>

// Function Prototype
int main( void );
int discount( int is_male, int price );



int main( void ) {
	int is_male, price;
	printf( "男性なら1女性なら2を入力。スペースを入力。商品の金額を入力。\n" );
	
	//ここに必要なscanf関数を記述する。想定外の入力についてはここでは無視する。
    scanf( "%d %d", &is_male, &price );
	
    int result = discount( is_male, price ); 
    
    printf("割引後の価格は%d円\n", result);
   	return 0;
}



int discount( int is_male, int price ) {
    
  //ここに必要な処理を記述する
  switch ( is_male ) {
    case 1:
      return ( price * 0.9 );
      break;
    case 2:
      return ( price * 0.8 );  
      break;
    default:
      return 0;
      break;
  }
}

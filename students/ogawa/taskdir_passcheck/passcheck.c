#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define SIZE_BUF 32



// Function Prototype
int main( void );
int check_pass( void );
void err_detail( void );



// Global Variable
char password[ SIZE_BUF ];
int ok_len = 0;   // 文字数8-20
int ok_low = 0;   // 小文字あり
int ok_up = 0;    // 大文字あり
int ok_num = 0;   // 数字あり
int ok_pun = 0;   // 記号あり



int main( void ) {

  while( 1 ) {


    printf( "パスワードを入力してください: " );
    fgets( password, sizeof( password ), stdin );

    if ( check_pass() ) {
      puts( "パスワードは合格です\n" );
      break;
    } else {
      puts( "パスワードは条件を満たしていません\n" );
      err_detail();
    }

    if ( ! strchr( password, '\n' ) ) { // 読み込んだパスワードに改行が含まれていないなら
      while( getchar() != '\n' ); // 入力バッファクリア
    }

  }

  return 0;

}



int check_pass( void ) {

// パスワード長
  if ( strlen( password ) >= 8 && strlen( password ) <= 20 ) {
    ok_len = 1;
  }

  // 内容チェック
  for ( int i = 0; i < strlen( password ); i++ ) {
    if ( islower( password[ i ] ) ) {
      ok_low = 1;
    }
    if ( isupper( password[ i ] ) ) {
      ok_up = 1;
    }
    if ( isdigit( password[ i ] ) ) {
      ok_num = 1;
    }
    if ( ispunct( password[ i ] ) ) {
      ok_pun = 1;
    }
  }

  if ( ok_len && ok_low && ok_up && ok_num && ok_pun ) {
    return 1;
  } else {
    return 0;
  }

}



void err_detail( void ) {
  puts( password );
  printf( "ok_len(8-20): %d(%d)\n", ok_len, strlen( password ) );
  printf( "ok_low: %d\n", ok_low );
  printf( "ok_up: %d\n", ok_up );
  printf( "ok_num: %d\n", ok_num );
  printf( "ok_pun: %d\n", ok_pun );
  return;
}

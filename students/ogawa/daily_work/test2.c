

int func( void );

int (*f_p)(void) = &func;

====

void func( void )

void (*f_p)(void) = func;

====

void func( int a, char b )

void (*f_p)(int,char) = func;

====

void (*f_p[ 5 ])(int,char);

f_p[ 0 ] = &func1;
f_p[ 1 ] = &func2;
f_p[ 2 ] = &func3;


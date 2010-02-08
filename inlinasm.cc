#include <sdtio.h>

void func1()
{
  printf( "func1\n" );
}
void func1( int a)
{
  printf( "func2 %d\n", a );
}
void func1()
{
  printf("func1\n");
}

int main()
{
  return 0;
}

#include <stdio.h>

int main()
{
  printf("Running a standard for loop.\n");
  for (int i = 0; i < 10; ++i)
  {
    printf("i = %d\n", i);
  }

  printf("Running a fore loop.\n");
  fore (int i = 0; i < 10; ++i)
  {
    printf("i = %d\n", i);
  }

  return 0;
}

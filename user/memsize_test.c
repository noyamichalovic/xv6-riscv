#include "kernel/types.h"
#include "user/user.h"

int
main(void)
{
  printf("Memory before malloc:      %d bytes\n", memsize());

  char *p = malloc(20 * 1024);

  printf("Memory after malloc(20k):  %d bytes\n", memsize());

  free(p);

  printf("Memory after free:         %d bytes\n", memsize());

  exit(0);
}

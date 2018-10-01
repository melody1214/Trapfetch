#include <stdio.h>
#include "test.h"

void sub()
{
#ifdef	X32
	printf("X32 is defined 3\n");
#endif
}

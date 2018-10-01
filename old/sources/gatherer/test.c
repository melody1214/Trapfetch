#include <stdio.h>
#include "test.h"

void numbers()
{
#ifdef	X32
	printf("X32 is defined 2\n");
#endif
}

int main()
{
#ifdef	X32
	printf("X32 is defined\n");
#endif

#undef	X32
	numbers();
	sub();

	return 0;
}


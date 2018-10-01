#include <stdio.h>

#define X32

void my_func()
{
#ifdef	X32
	printf("2\n");
#endif
}

int main()
{
#undef X32
#ifdef	X32
	printf("1\n");
#endif

#define X32
	my_func();
	return 0;
}

#include <stdio.h>

int main(int argc, char *argv[]) {
    unsigned long long int a = 0x7ffff4761c66;
    void *b = (void *)0x7ffff4761c66;

    printf("unsigned long long int a: %p, a - 1: %p\n", (void *)a, (void *)(a-1));
    printf("void *b: %p, b - 1: %p\n", b, b-1);
    
    return 0;
}
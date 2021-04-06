#include <stdio.h>
#include <unistd.h>

#define SZ_128K 0x00020000

int main(int argc, char *argv[]) {
    int max_readahead_size;
    int page_size;

    max_readahead_size = SZ_128K;
    page_size = getpagesize();

    printf("max readahead size of this system is %d\n", max_readahead_size * page_size);
    return 0;
}
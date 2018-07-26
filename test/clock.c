#define _POSIX_C_COURSE

#include <stdio.h>
#include <time.h>
#include <unistd.h>

int main(void)
{
    struct timespec ts_coarse;
    struct timespec ts;
    unsigned long long clk_mntn;
    int i;

   	clock_gettime(CLOCK_REALTIME, &ts);

   	clk_mntn = (unsigned long long)ts.tv_nsec;
   

       printf("MONOTONIC : %llu\n", clk_mntn);
}

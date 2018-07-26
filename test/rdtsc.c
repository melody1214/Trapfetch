#include "rdtsc.h"
#include <stdio.h>
#include <stdlib.h>

/*
void FunctionToTest()
{
    printf("hello world\n");
}
*/

uint32_t get_cycles_to_nsec_scale(unsigned int tsc_frequency_khz)
{
    return (uint32_t)((1000000) << 10) / (uint32_t)tsc_frequency_khz;
}

uint64_t cycles_to_nsec(uint64_t cycles, uint32_t scale_factor)
{
    return (cycles * scale_factor) >> 10;
}

int main()
{
    const int numtests = 10;
    const int pmc_num = 0x40000001;
    uint64_t clockcounts[numtests];
    uint64_t pmccounts[numtests];
    uint64_t clock1, clock2, pmc1, pmc2;
    int i;
	/*
    for (i = 0; i < numtests; i++)
    {
        serialize();
        clock1 = readtsc();
        pmc1 = readpmc(pmc_num);
	*/
        //FunctionToTest();

        // serialize();
        // clock2 = readtsc();
        // pmc2 = readpmc(pmc_num);
        // clockcounts[i] = clock2 - clock1;
        // pmccounts[i] = pmc2 - pmc1;
   //     clockcounts[i] = clock1;
    //    pmccounts[i] = pmc1;
    //}
	//
    printf("\nTest results\nclock counts    PMC counts");
    uint32_t scale = get_cycles_to_nsec_scale(3618373);

  	system("cat /proc/uptime");
	clock1 = readtsc();

    //for (i = 0; i < numtests; i++)
    //{
        uint64_t clock = cycles_to_nsec(clock1, scale);
  //      uint64_t pmc = cycles_to_nsec(pmccounts[i], scale);
        printf("\n%10ld", clock);
    //}
    printf("\n");

    return 0;
}

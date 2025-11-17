#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#include "game_of_life.h"

#define PROGRESS_BAR 100

int gameOfLife(void *n)
{
	uint8_t *cell = n;
	uint8_t sum = 0;
	for(int i = 1; i <= 8; i++)
		sum += cell[i];
	cell[0] = (cell[0] | sum) == 3;
	return 0;
}

int main(int argc, char *argv[])
{
	if(argc != 4)
    {
        printf("usage: %s file_in file_out num_of_iters\n", argv[0]);
        return 1;
    }
    char *p;
    errno = 0;
    long itersNumber = strtol(argv[3], &p, 10);
    if (errno != 0 || *p != '\0' || itersNumber > 1000000 || itersNumber < 0)
    {
        printf("Error: wrong number of iterations\n");
        return 1;
    }

    printf("Loading input file: %s\n", argv[1]);
    int flag = CAT_InitSimulator((char *)argv[1]);
    if (flag)
    {
        printf("Error: can't read input file: %s\n", argv[1]);
        return 1;
    }

    printf("Simulator started for %ld iterations with %d thread(s)\n", itersNumber, CAT_GetNumThreads()); fflush(stdout);
	int dotsNumber = itersNumber > PROGRESS_BAR ? PROGRESS_BAR : itersNumber;
   	printf("["); fflush(stdout);
   	for (int i = 0; i < dotsNumber; i++)
   		printf("."); fflush(stdout);
   	printf("]\r["); fflush(stdout);

	long long t1, t2;
	double tDiff;
	struct timespec curTime;
	clock_gettime(CLOCK_BOOTTIME, &curTime);
	t1 = curTime.tv_sec * 1000000000 + curTime.tv_nsec;

    for (int i = 0; i < itersNumber; i++)
    {
    	if(i % (itersNumber / dotsNumber) == 0)
    		printf("#"); fflush(stdout);

        flag = CAT_Iterate(gameOfLife);
        if (flag)
        {
            printf("\nError: %d, iteration: %d\n", flag, i);
            return 1;
        }
    }

	clock_gettime(CLOCK_BOOTTIME, &curTime);
	t2 = curTime.tv_sec * 1000000000 + curTime.tv_nsec;
	tDiff = (double) (t2 - t1) / 1000000000.0;

    printf("]\nTime = %g s\n", tDiff); fflush(stdout);

    printf("Saving output file: %s\n", argv[2]);
    flag = CAT_FinalizeSimulator((char *)argv[2]);
    if (flag)
    {
        printf("Error: can't write output file: %s\n", argv[2]);
        return 1;
    }

    printf("Simulator finished\n");
    return 0;
}


//./catmdl_game_of_life_sim initial_state.dat state.dat 100
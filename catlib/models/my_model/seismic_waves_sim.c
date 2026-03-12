#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "catlib.h"
#include "seismic_waves.h"

int propogation(void *n)
{
    cellBody *cell = (cellBody *)n;
    int sum = -cell[0].compression;
    for (int i = 1; i <= 8; i++)
    {
        sum += cell[i].compression / 8;
    }
    cell[0].compression = sum;
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("usage: %s file_in file_out num_of_iters \n", argv[0]);
        return 1;
    }

    long itersNumber = strtol(argv[3], NULL, 10);

    printf("Loading: %s\n", argv[1]);
    if (CAT_InitSimulator(argv[1]) != 0)
    {
        printf("Error reading input\n");
        return 1;
    }

    CAT_Index size = CAT_GetArraySize();
    int I = size.i;
    int J = size.j;
    printf("Grid: %dx%d, Iters: %ld\n", I, J, itersNumber);

    clock_t t1 = clock();

    for (int i = 0; i < itersNumber; i++)
    {
        if (CAT_Iterate(propogation))
        {
            printf("Error at iteration %d\n", i);
            break;
        }
    }

    clock_t t2 = clock();
    double tDiff = (double)(t2 - t1) / CLOCKS_PER_SEC;
    printf("Time = %g s\n", tDiff);

    printf("Saving: %s\n", argv[2]);
    if (CAT_FinalizeSimulator(argv[2]) != 0)
    {
        printf("Error writing output\n");
        return 1;
    }

    return 0;
}
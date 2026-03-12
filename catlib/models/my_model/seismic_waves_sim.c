#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "catlib.h"
#include "seismic_waves.h"

int propogation(void *n)
{
    cellBody *cell = (cellBody *)n;
    cellBody left_cell = cell[7];
    cellBody right_cell = cell[3];
    cellBody up_cell = cell[1];
    cellBody down_cell = cell[5];

    int sum = -cell[0].compression;
    int compression_direction_num = 1;

    if (left_cell.time_iteration < cell->time_iteration)
    {
        sum += left_cell.compression / 4; // пока что делим на 4 так как непонятно как именно распределять энергию (пропорционально сжатиию?)
    }
    if (right_cell.time_iteration < cell->time_iteration)
    {
        sum += right_cell.compression / 4;
    }
    if (up_cell.time_iteration < cell->time_iteration)
    {
        sum += up_cell.compression / 4;
    }
    if (down_cell.time_iteration < cell->time_iteration)
    {
        sum += down_cell.compression / 4;
    }
    // for (int i = 1; i <= 8; i += 2)
    // {
    //     sum += cell[i].compression / 4;
    // }
    if (sum > 0) // если в клетку пришло сжатие, то волна движется в сторону этой клетки
    {
        cell[0].time_iteration = CAT_GetIterationsDone();
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
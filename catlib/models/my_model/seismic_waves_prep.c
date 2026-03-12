#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include "catlib.h"
#include "seismic_waves.h"

const char fileName[] = "initial_state.dat";

int main(int argc, char *argv[])
{
    const int globalSize = 0;
    const double Kl = 1.0;

    CAT_Coord coordMax;
    coordMax.x = 511.0;
    coordMax.y = 511.0;

    CAT_Coord realCoord = CAT_InitPreprocessor(cellSize, globalSize, Kl, coordMax);

    if (realCoord.x * realCoord.y == 0.0)
    {
        printf("Error: can't initialize preprocessor (returned 0 size)\n");
        return 1;
    }

    CAT_Index indexMax = CAT_GetArraySize();
    int I = indexMax.i;
    int J = indexMax.j;

    printf("Grid init: %.0f x %.0f -> Integer grid: %d x %d\n",
           realCoord.x, realCoord.y, I, J);

    cellBody cell;
    int center_i = I / 2;
    int center_j = J / 2;
    int radius = 32;

    for (int i = 0; i < I; i++)
    {
        for (int j = 0; j < J; j++)
        {
            cell.velocity = 340;
            cell.compression = 0;
            cell.time_iteration = INT32_MAX;

            int dx = i - center_i;
            int dy = j - center_j;
            if (dx * dx + dy * dy <= radius * radius)
            {
                cell.compression = 30000;
                cell.time_iteration = 0;
            }
            CAT_PutCell((char *)&cell, i, j);
        }
    }

    if (CAT_FinalizePreprocessor((char *)fileName) != 0)
    {
        printf("Error: can't write output file: %s\n", fileName);
        return 1;
    }

    printf("Success! Initial state created: %s\n", fileName);
    return 0;
}
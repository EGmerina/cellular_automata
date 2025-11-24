#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "catlib.h"
#include "seismic_waves.h"

int collision(void *n)
{
    cellBody *cell = (cellBody *)n;
    uint8_t state = cell->bits;
    uint8_t new_state = state;

    uint8_t p = state & P_MASK;

    if (p == (P_RIGHT | P_LEFT))
    {
        new_state &= ~P_MASK;
        new_state |= (P_UP | P_DOWN);
    }

    else if (p == (P_UP | P_DOWN))
    {
        new_state &= ~P_MASK;
        new_state |= (P_RIGHT | P_LEFT);
    }

    uint8_t n_part = state & N_MASK;

    if (n_part == (N_RIGHT | N_LEFT))
    {
        new_state &= ~N_MASK;
        new_state |= (N_UP | N_DOWN);
    }

    else if (n_part == (N_UP | N_DOWN))
    {
        new_state &= ~N_MASK;
        new_state |= (N_RIGHT | N_LEFT);
    }

    cell->bits = new_state;
    return 0;
}


void propagation_step(int width, int height)
{
   
    uint8_t *next_grid = (uint8_t *)calloc(width * height, sizeof(uint8_t));
    if (!next_grid)
        exit(1);

    cellBody cell;

    
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            CAT_GetCell((char *)&cell, i, j);
            uint8_t b = cell.bits;
            if (b == 0)
                continue;


            int i_next = i + 1;
            int i_prev = i - 1;
            int j_next = j + 1;
            int j_prev = j - 1;

           
            if ((b & P_RIGHT) && (i_next < width))
                next_grid[j * width + i_next] |= P_RIGHT;
            if ((b & P_LEFT) && (i_prev >= 0))
                next_grid[j * width + i_prev] |= P_LEFT;
            if ((b & P_DOWN) && (j_next < height))
                next_grid[j_next * width + i] |= P_DOWN;
            if ((b & P_UP) && (j_prev >= 0))
                next_grid[j_prev * width + i] |= P_UP;

           
            if ((b & N_RIGHT) && (i_next < width))
                next_grid[j * width + i_next] |= N_RIGHT;
            if ((b & N_LEFT) && (i_prev >= 0))
                next_grid[j * width + i_prev] |= N_LEFT;
            if ((b & N_DOWN) && (j_next < height))
                next_grid[j_next * width + i] |= N_DOWN;
            if ((b & N_UP) && (j_prev >= 0))
                next_grid[j_prev * width + i] |= N_UP;
        }
    }

   
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            cell.bits = next_grid[j * width + i];
            CAT_PutCell((char *)&cell, i, j);
        }
    }

    free(next_grid);
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("usage: %s file_in file_out num_of_iters\n", argv[0]);
        return 1;
    }

    long itersNumber = strtol(argv[3], NULL, 10);

    printf("Loading: %s\n", argv[1]);
    if (CAT_InitSimulator(argv[1]) != 0)
    {
        printf("Error: can't read input file\n");
        return 1;
    }

    CAT_Index size = CAT_GetArraySize();
    int I = size.i;
    int J = size.j;

    printf("Grid: %dx%d, Iters: %ld\n", I, J, itersNumber);

    clock_t t1 = clock();

    for (int i = 0; i < itersNumber; i++)
    {
        int flag = CAT_Iterate(collision);
        if (flag)
        {
            printf("\nError: %d, iteration: %d\n", flag, i);
            return 1;
        }

        propagation_step(I, J);
    }

    clock_t t2 = clock();
    double tDiff = (double)(t2 - t1) / CLOCKS_PER_SEC;
    printf("\nTime = %g s\n", tDiff);

    printf("Saving: %s\n", argv[2]);
    if (CAT_FinalizeSimulator(argv[2]) != 0)
    {
        printf("Error writing output\n");
        return 1;
    }

    return 0;
}
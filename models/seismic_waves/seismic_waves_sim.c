#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#include "seismic_waves.h"

#define PROGRESS_BAR 100

int seismicCollision(void *neighbors)
{
    uint8_t *center = (uint8_t *)neighbors + 4 * cellSize; // центральная ячейка

    uint8_t current_state = *center;
    uint8_t new_state = 0;

  
    // Если две противоположные положительные частицы - они поворачивают на 90 градусов
    // Аналогично для отрицательных

    
    if ((current_state & POS_RIGHT) && (current_state & POS_LEFT))
    {
        new_state |= POS_UP | POS_DOWN;
    }
    else if ((current_state & POS_UP) && (current_state & POS_DOWN))
    {
        new_state |= POS_RIGHT | POS_LEFT; 
    }
    else
    {
        new_state |= current_state & ALL_POSITIVE;
    }

   
    if ((current_state & NEG_RIGHT) && (current_state & NEG_LEFT))
    {
        new_state |= NEG_UP | NEG_DOWN;
    }
    else if ((current_state & NEG_UP) && (current_state & NEG_DOWN))
    {
        new_state |= NEG_RIGHT | NEG_LEFT;
    }
    else
    {
        new_state |= current_state & ALL_NEGATIVE;
    }

    *center = new_state;
    return 0;
}


void transportParticles()
{
    int width, height;
    CAT_Index size = CAT_GetArraySize();
    width = size.i;
    height = size.j;

  
    uint8_t *new_grid = malloc(width * height * sizeof(uint8_t));

  
    for (int i = 0; i < width * height; i++)
    {
        new_grid[i] = 0;
    }

 
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            cellBody cell;
            CAT_GetCell(&cell, i, j);

           
            if (cell.directions & POS_RIGHT && i < width - 1)
                new_grid[(i + 1) * height + j] |= POS_RIGHT;
            if (cell.directions & POS_UP && j < height - 1)
                new_grid[i * height + (j + 1)] |= POS_UP;
            if (cell.directions & POS_LEFT && i > 0)
                new_grid[(i - 1) * height + j] |= POS_LEFT;
            if (cell.directions & POS_DOWN && j > 0)
                new_grid[i * height + (j - 1)] |= POS_DOWN;

          
            if (cell.directions & NEG_RIGHT && i < width - 1)
                new_grid[(i + 1) * height + j] |= NEG_RIGHT;
            if (cell.directions & NEG_UP && j < height - 1)
                new_grid[i * height + (j + 1)] |= NEG_UP;
            if (cell.directions & NEG_LEFT && i > 0)
                new_grid[(i - 1) * height + j] |= NEG_LEFT;
            if (cell.directions & NEG_DOWN && j > 0)
                new_grid[i * height + (j - 1)] |= NEG_DOWN;
        }
    }

   
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            cellBody cell;
            cell.directions = new_grid[i * height + j];
            CAT_PutCell(&cell, i, j);
        }
    }

    free(new_grid);
}

int main(int argc, char *argv[])
{
    if (argc != 4)
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

    printf("Seismic P-wave simulator started for %ld iterations\n", itersNumber);
    fflush(stdout);

    int dotsNumber = itersNumber > PROGRESS_BAR ? PROGRESS_BAR : itersNumber;
    printf("[");
    for (int i = 0; i < dotsNumber; i++)
        printf(".");
    printf("]\r[");
    fflush(stdout);

    clock_t t1, t2;
    double tDiff;

    t1 = clock(); 

    for (int iter = 0; iter < itersNumber; iter++)
    {
        if (dotsNumber > 0 && iter % (itersNumber / dotsNumber) == 0)
            printf("#");
        fflush(stdout);

        
        flag = CAT_Iterate(seismicCollision);
        if (flag)
        {
            printf("\nError: %d, iteration: %d\n", flag, iter);
            return 1;
        }

       
        transportParticles();
    }

    t2 = clock(); 
    tDiff = (double)(t2 - t1) / CLOCKS_PER_SEC;

    printf("]\nTime = %g s\n", tDiff);
    fflush(stdout);

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
#include <stdio.h>
#include <stdlib.h>
#include "game_of_life.h"

const char inputFileName[] = "initial_state.dat";

void putGlider(int i, int j)
{
	cellBody cell;
    cell.Value = 1;
    CAT_PutCell(&cell, i    , j + 1);
    CAT_PutCell(&cell, i + 1, j + 2);
    CAT_PutCell(&cell, i + 2, j + 2);
    CAT_PutCell(&cell, i + 2, j + 1);
    CAT_PutCell(&cell, i + 2, j    );
}

int main()
{
    const int globalSize = 0;
    const double Kl = 1.0;
    int I, J;
    CAT_Coord coordMax;

    cellBody *cell;
    cell = malloc(cellSize);
    if (cell == NULL)
        return -1;

    cell->Value = 0;
    double X = coordMax.x = 999;
    double Y = coordMax.y = 999;

    coordMax = CAT_InitPreprocessor(cellSize, globalSize, Kl, coordMax);
    if (coordMax.x * coordMax.y == 0.0)
    {
        printf("Error: can't initialize preprocessor\n");
        return 1;
    }
    
    CAT_Index indexMax = CAT_GetArraySize();
    I = indexMax.i;
    J = indexMax.j;
	printf("%f x %f => %d x %d => %f x %f\n", X, Y, I, J, coordMax.x, coordMax.y);

    cell->Value = 0;
    for (int i = 0; i < I; i++)
        for (int j = 0; j < J; j++)
            CAT_PutCell(cell, i, j);

    for (int i = 0; i < I - 4; i += 5)
        for (int j = 0; j < J - 4; j += 5)
            putGlider(i, j);

    int flag = CAT_FinalizePreprocessor((char *)inputFileName);
    if (flag)
    {
        printf("Error: can't write output file: %s\n", inputFileName);
        return 1;
    }

    return 0;
}

//./catmdl_game_of_life_prep 
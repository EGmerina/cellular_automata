#include <stdio.h>
#include <stdlib.h>
#include "fhpmp.hpp"

#define CONVENTIONAL 0
#define INLET 1
#define OUTLET 2
#define WALL 15

const char fileName[] = "initial_state.dat";

void set_cell_type(int i, int j, int type)
{
    hexCell cell;
    cell.countCellsDirection0 = 0;
    cell.countCellsDirection1 = 0;
    cell.countCellsDirection2 = 0;
    cell.countCellsDirection3 = 0;
    cell.countCellsDirection4 = 0;
    cell.countCellsDirection5 = 0;
    cell.cellType = type;
    CAT_PutCell(&cell, i, j);
}

int main()
{
    const int globalSize = 0;
    const double Kl = 1.0;
    CAT_Coord coordMax;
    double X = coordMax.x = 200;
    double Y = coordMax.y = 100;

    coordMax = CAT_InitPreprocessor(cellSize, globalSize, Kl, coordMax);
    if (coordMax.x * coordMax.y == 0.0)
    {
        printf("Error: can't initialize preprocessor\n");
        return 1;
    }

    CAT_Index indexMax = CAT_GetArraySize();
    int I = indexMax.i;
    int J = indexMax.j;
    printf("%f x %f => %d x %d => %f x %f\n", X, Y, I, J, coordMax.x, coordMax.y);

    for (int i = 0; i < I; i++)
        for (int j = 0; j < J; j++)
            set_cell_type(i, j, CONVENTIONAL);

    for (int j = 0; j < J; j++)
    {
        set_cell_type(0, j, INLET);
        set_cell_type(I - 1, j, OUTLET);
    }

    for (int i = 0; i < I; i++)
    {
        set_cell_type(i, 0, WALL);
        set_cell_type(i, J - 1, WALL);
    }

    for (int j = J / 3; j < 2 * J / 3; j++)
        set_cell_type(j, J - j, WALL);

    for (int j = 0; j < J / 3; j++)
        set_cell_type(j + 20, j + 20, WALL);

    int flag = CAT_FinalizePreprocessor((char *)fileName);
    if (flag)
    {
        printf("Error: can't write output file: %s\n", fileName);
        return 1;
    }

    return 0;
}

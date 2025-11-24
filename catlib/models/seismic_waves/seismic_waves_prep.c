#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "seismic_waves.h"

const char inputFileName[] = "initial_state.dat";

void createPointSource(int cx, int cy, int radius)
{
    cellBody cell;

    for (int i = cx - radius; i <= cx + radius; i++)
    {
        for (int j = cy - radius; j <= cy + radius; j++)
        {

            cell.directions = 0;

            if ((i - cx) * (i - cx) + (j - cy) * (j - cy) <= radius * radius)
            {
                cell.directions = POS_RIGHT | POS_LEFT | POS_DOWN | POS_UP;
            }

            CAT_PutCell(&cell, i, j);
        }
    }
}

int main()
{
    const int globalSize = 0;
    const double Kl = 1.0;
    int width, height;
    CAT_Coord coordMax;

    cellBody *cell;
    cell = malloc(cellSize);
    if (cell == NULL)
        return -1;

    coordMax.x = 1023;
    coordMax.y = 1023;

    coordMax = CAT_InitPreprocessor(cellSize, globalSize, Kl, coordMax);
    if (coordMax.x * coordMax.y == 0.0)
    {
        printf("Error: can't initialize preprocessor\n");
        return 1;
    }

    CAT_Index indexMax = CAT_GetArraySize();
    width = indexMax.i;
    height = indexMax.j;
    printf("Grid size: %d x %d\n", width, height);

    // Инициализация сбалансированной среды
    // cell->directions = 0; // Пустота
    cell->directions = ALL_POSITIVE | ALL_NEGATIVE; // нейтральное состояние
    for (int i = 0; i < width; i++)
        for (int j = 0; j < height; j++)
            CAT_PutCell(cell, i, j);

    // Создание точечного источника в центре
    createPointSource(width / 2, height / 2, 32);

    // Альтернативно: создание слоистой модели
    // createEarthModel(width, height, height/4);

    int flag = CAT_FinalizePreprocessor((char *)inputFileName);
    if (flag)
    {
        printf("Error: can't write output file: %s\n", inputFileName);
        return 1;
    }

    printf("Initial state created successfully\n");
    free(cell);
    return 0;
}

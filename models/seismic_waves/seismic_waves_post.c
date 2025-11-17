#include <stdio.h>
#include <stdlib.h>
#include "seismic_waves.h"

int main(int argc, char *argv[])
{
    int width, height;
    char filename[1000];
    FILE *file;
    cellBody *cell;
    
    if(argc != 2)
    {
        printf("usage: %s file_in\n", argv[0]);
        return 1;
    }
    
    printf("Seismic postprocessor started\n");
    cell = malloc(cellSize);
    if (cell == NULL)
    {
        printf("Error: can't allocate memory\n");
        return 1;
    }
    
    printf("Loading input file: %s\n", argv[1]);
    int flag = CAT_InitPostprocessor((char *)argv[1]);
    if (flag)
    {
        printf("Error: can't read input file: %s\n", argv[1]);
        return 1;
    }

    CAT_Index index = CAT_GetArraySize();
    width = index.i;
    height = index.j;
    printf("Array size: %d x %d\n", width, height);

    sprintf(filename, "%s.pgm", argv[1]);
    file = fopen(filename, "w");
    if (file == NULL)
    {
        printf("Error: can't write output file: %s\n", filename);
        return 1;
    }
    
    // PGM файл с градациями серого
    fprintf(file, "P2\n%d %d\n15\n", width, height);

    printf("Writing output file: %s\n", filename);
    
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            CAT_GetCell(cell, i, j);
            
            // Вычисляем волновое поле как разность положительных и отрицательных частиц
            int positive_count = __builtin_popcount(cell->directions & ALL_POSITIVE);
            int negative_count = __builtin_popcount(cell->directions & ALL_NEGATIVE);
            int wave_value = positive_count - negative_count + 4; // смещение для положительных значений
            
            fprintf(file, "%d ", wave_value);
        }
        fprintf(file, "\n");
    }
    
    fclose(file);
    CAT_FinalizePostprocessor();
    printf("Postprocessor finished\n");
    free(cell);
    return 0;
}
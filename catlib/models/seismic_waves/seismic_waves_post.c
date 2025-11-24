#include <stdio.h>
#include <stdlib.h>
#include "catlib.h"
#include "seismic_waves.h"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("usage: %s file_in\n", argv[0]);
        return 1;
    }

    if (CAT_InitPostprocessor(argv[1]) != 0)
    {
        printf("Error reading input file\n");
        return 1;
    }

    CAT_Index size = CAT_GetArraySize();
    int width = size.i;
    int height = size.j;

    char filename[1024];
    sprintf(filename, "%s.pgm", argv[1]);
    FILE *file = fopen(filename, "w");
    if (!file)
        return 1;

    fprintf(file, "P2\n%d %d\n255\n", width, height);

    cellBody cell;
    for (int j = 0; j < height; j++)
    { // PGM usually row-major
        for (int i = 0; i < width; i++)
        {
            CAT_GetCell((char *)&cell, i, j);

            int pos = __builtin_popcount(cell.directions & ALL_POSITIVE);
            int neg = __builtin_popcount(cell.directions & ALL_NEGATIVE);

            // Визуализация плотности (амплитуды)
            int val = 128 + (pos - neg) * 20;
            if (val < 0)
                val = 0;
            if (val > 255)
                val = 255;

            fprintf(file, "%d ", val);
        }
        fprintf(file, "\n");
    }

    fclose(file);
    CAT_FinalizePostprocessor(NULL); // Аргумент нужен, даже если не сохраняем заголовок
    return 0;
}
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
        printf("Error reading input\n");
        return 1;
    }

    CAT_Index size = CAT_GetArraySize();
    int width = size.i;
    int height = size.j;

    char filename[256];
    sprintf(filename, "%s.pgm", argv[1]);
    FILE *f = fopen(filename, "w");

    fprintf(f, "P2\n%d %d\n255\n", width, height);

    cellBody cell;

    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            CAT_GetCell((char *)&cell, i, j);

            // Чтобы отобразить это в 0-255, делаем смещение.
            // 128 - ноль. Больше - положительная амплитуда, меньше - отрицательная.
            int val = (cell.compression) / 30000.0 * 255 + 128;

            if (val < 0)
                val = 0;
            if (val > 255)
                val = 255;

            fprintf(f, "%d ", val);
        }
        fprintf(f, "\n");
    }

    fclose(f);
    CAT_FinalizePostprocessor(NULL);
    printf("Generated %s\n", filename);
    return 0;
}

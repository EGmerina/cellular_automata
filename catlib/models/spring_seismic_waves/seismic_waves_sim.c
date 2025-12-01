#include <stdio.h>
#include <stdlib.h>
#include "catlib.h"
#include "seismic_waves.h"

void wave_propagation_step(int width, int height)
{

    double *next_u = (double *)malloc(width * height * sizeof(double));
    if (!next_u)
        exit(1);

    cellBody center;
    cellBody neighbors[4]; // right, up, left, down

    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            CAT_GetCell((char *)&center, i, j);

            double u_right = 0, u_up = 0, u_left = 0, u_down = 0;

            // получаем соседей если они есть
            if (i + 1 < width)
            {
                CAT_GetCell((char *)&neighbors[0], i + 1, j);
                u_right = neighbors[0].u;
            }

            if (j + 1 < height)
            {
                CAT_GetCell((char *)&neighbors[1], i, j + 1);
                u_up = neighbors[1].u;
            }

            if (i - 1 >= 0)
            {
                CAT_GetCell((char *)&neighbors[2], i - 1, j);
                u_left = neighbors[2].u;
            }

            if (j - 1 >= 0)
            {
                CAT_GetCell((char *)&neighbors[3], i, j - 1);
                u_down = neighbors[3].u;
            }


            // Дискретный Лапласиан (сумма разниц с соседями)
            double laplacian = (u_right + u_left + u_up + u_down - 4.0 * center.u);

            // Волновое уравнение
            double u_next_val = 2.0 * center.u - center.u_prev + (center.velocity_sqr * laplacian);

            // затухание
            u_next_val *= center.damping;

            next_u[j * width + i] = u_next_val;
        }
    }

   //обновляем
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            CAT_GetCell((char *)&center, i, j);

            center.u_prev = center.u;         
            center.u = next_u[j * width + i]; 

            CAT_PutCell((char *)&center, i, j);
        }
    }

    free(next_u);
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("usage: %s file_in file_out num_of_iters\n", argv[0]);
        return 1;
    }

    long itersNumber = strtol(argv[3], NULL, 10);

    if (CAT_InitSimulator(argv[1]) != 0)
    {
        printf("Error reading input\n");
        return 1;
    }

    CAT_Index size = CAT_GetArraySize();
    int I = size.i;
    int J = size.j;

    printf("Seismic Simulation: %dx%d, %ld ticks\n", I, J, itersNumber);

    for (int k = 0; k < itersNumber; k++)
    {

        wave_propagation_step(I, J);

        if (k % 50 == 0)
            printf("Step %d...\n", k);
    }

    if (CAT_FinalizeSimulator(argv[2]) != 0)
    {
        printf("Error writing output\n");
        return 1;
    }

    return 0;
}
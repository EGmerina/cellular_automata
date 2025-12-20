#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "catlib.h"
#include "seismic_waves.h"

const char fileName[] = "initial_state.dat";

void set_cell(int i, int j, int type, uint8_t initial_bits)
{
    cellBody cell;
    cell.type = type;
    cell.bits = initial_bits;
    CAT_PutCell((char *)&cell, i, j);
}

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

    srand(time(NULL));

    cellBody cell;
    int center_i = I / 2;
    int center_j = J / 2;
    int radius = 32;

    for (int i = 0; i < I; i++)
    {
        for (int j = 0; j < J; j++)
        {
            cell.bits = 0;

            if (rand() > (RAND_MAX / 2))
            {
                cell.bits |= P_RIGHT;
                cell.bits |= N_RIGHT;
            }

            if (rand() > (RAND_MAX / 2))
            {
                cell.bits |= P_UP;
                cell.bits |= N_UP;
            }

            if (rand() > (RAND_MAX / 2))
            {
                cell.bits |= P_LEFT;
                cell.bits |= N_LEFT;
            }

            if (rand() > (RAND_MAX / 2))
            {
                cell.bits |= P_DOWN;
                cell.bits |= N_DOWN;
            }

            int dx = i - center_i;
            int dy = j - center_j;
            if (dx * dx + dy * dy <= radius * radius)
            {

                cell.bits |= (P_RIGHT | P_UP | P_LEFT | P_DOWN);

                cell.bits &= P_MASK;
                set_cell(i, j, TYPE_SOURCE, cell.bits);
            }
            else
            {
                set_cell(i, j, TYPE_MEDIUM, cell.bits);
            }

            //  int dx = i - center_i;
            // int dy = j - center_j;
            // if (dy * dy <= radius * radius)
            // {

            //     cell.bits |= (P_RIGHT | P_UP | P_LEFT | P_DOWN);

            //     cell.bits &= P_MASK;
            //     set_cell(i, j, TYPE_SOURCE, cell.bits);
            // }
            // else
            // {
            //     set_cell(i, j, TYPE_MEDIUM, cell.bits);
            // }
        }
    }

    // Верхняя граница - свободная
    for (int i = 0; i < I; i++)
    {
        set_cell(i, 0, TYPE_WALL_FREE, 0);
        set_cell(i, J - 1, TYPE_WALL_SOLID, 0); // Низ (твердое дно)
                                                // set_cell(i, J - 1, TYPE_WALL_FREE, 0);
    }

    // Боковые границы - твердые
    for (int j = 0; j < J; j++)
    {
        set_cell(0, j, TYPE_WALL_SOLID, 0);
        set_cell(I - 1, j, TYPE_WALL_SOLID, 0);
    }

    if (CAT_FinalizePreprocessor((char *)fileName) != 0)
    {
        printf("Error: can't write output file: %s\n", fileName);
        return 1;
    }

    printf("Success! Initial state created: %s\n", fileName);
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "catlib.h"
#include "seismic_waves.h"

int is_oscillation = 0;

cellBody collision_medium(cellBody cell)
{
    uint8_t state = cell.bits;
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

    cell.bits = new_state;
    return cell;
}

//  Твердая стенка: отражает P->P, N->N, меняет направление на 180
cellBody collision_wall_solid(cellBody cell)
{
    uint8_t in = cell.bits;
    uint8_t out = 0;

    if (in & P_RIGHT)
        out |= P_LEFT;
    if (in & P_LEFT)
        out |= P_RIGHT;
    if (in & P_UP)
        out |= P_DOWN;
    if (in & P_DOWN)
        out |= P_UP;

    if (in & N_RIGHT)
        out |= N_LEFT;
    if (in & N_LEFT)
        out |= N_RIGHT;
    if (in & N_UP)
        out |= N_DOWN;
    if (in & N_DOWN)
        out |= N_UP;

    cell.bits = out;
    return cell;
}

//  Свободная граница: отражает P->N, N->P, меняет направление на 180
cellBody collision_wall_free(cellBody cell)
{
    uint8_t in = cell.bits;
    uint8_t out = 0;

    if (in & P_RIGHT)
        out |= N_LEFT;
    if (in & P_LEFT)
        out |= N_RIGHT;
    if (in & P_UP)
        out |= N_DOWN;
    if (in & P_DOWN)
        out |= N_UP;

    if (in & N_RIGHT)
        out |= P_LEFT;
    if (in & N_LEFT)
        out |= P_RIGHT;
    if (in & N_UP)
        out |= P_DOWN;
    if (in & N_DOWN)
        out |= P_UP;

    cell.bits = out;
    return cell;
}

cellBody collision_source(cellBody cell)
{

    if (is_oscillation)
    {
        cell.bits = 0;
        cell.bits = (P_RIGHT | P_LEFT | P_UP | P_DOWN);
    }

    return cell;
}

int collision(void *n)
{
    cellBody *cell = (cellBody *)n;

    switch (cell->type)
    {
    case TYPE_MEDIUM:
        *cell = collision_medium(*cell);
        break;
    case TYPE_WALL_SOLID:
        *cell = collision_wall_solid(*cell);
        break;
    case TYPE_WALL_FREE:
        *cell = collision_wall_free(*cell);
        break;
    case TYPE_SOURCE:
        *cell = collision_source(*cell);
        break;
    default:
        *cell = collision_medium(*cell);
        break;
    }
    return 0;
}

// int propagation(void *n)
// {
//     cellBody *cell = (cellBody *)n;

//     cellBody *cell_up = cell + 1;
//     cellBody *cell_right = cell + 2;
//     cellBody *cell_down = cell + 3;
//     cellBody *cell_left = cell + 4;

//     if (cell_right && (cell_right->bits & P_LEFT)) // тут мы работает в пределах одной клетки
//         cell->bits |= P_LEFT;
//     if (cell_left && (cell_left->bits & P_RIGHT))
//         cell->bits |= P_RIGHT;
//     if (cell_up && (cell_up->bits & P_DOWN))
//         cell->bits |= P_DOWN;
//     if (cell_down && (cell_down->bits & P_UP))
//         cell->bits |= P_UP;

//     if (cell_right && (cell_right->bits & N_LEFT))
//         cell->bits |= N_LEFT;
//     if (cell_left && (cell_left->bits & N_RIGHT))
//         cell->bits |= N_RIGHT;
//     if (cell_up && (cell_up->bits & N_DOWN))
//         cell->bits |= N_DOWN;
//     if (cell_down && (cell_down->bits & N_UP))
//         cell->bits |= N_UP;

//     return 0;
// }

void propagation_step(int width, int height, cellBody *next_grid)
{
    memset(next_grid, 0, width * height * sizeof(cellBody));

    cellBody cell;

    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            CAT_GetCell((char *)&cell, i, j);

            uint8_t b = cell.bits;
            next_grid[j * width + i].type = cell.type;

            if (b == 0)
                continue;

            int i_next = i + 1;
            int i_prev = i - 1;
            int j_next = j + 1;
            int j_prev = j - 1;

            if ((b & P_RIGHT) && (i_next < width))
                next_grid[j * width + i_next].bits |= P_RIGHT;
            if ((b & P_LEFT) && (i_prev >= 0))
                next_grid[j * width + i_prev].bits |= P_LEFT;
            if ((b & P_DOWN) && (j_next < height))
                next_grid[j_next * width + i].bits |= P_DOWN;
            if ((b & P_UP) && (j_prev >= 0))
                next_grid[j_prev * width + i].bits |= P_UP;

            if ((b & N_RIGHT) && (i_next < width))
                next_grid[j * width + i_next].bits |= N_RIGHT;
            if ((b & N_LEFT) && (i_prev >= 0))
                next_grid[j * width + i_prev].bits |= N_LEFT;
            if ((b & N_DOWN) && (j_next < height))
                next_grid[j_next * width + i].bits |= N_DOWN;
            if ((b & N_UP) && (j_prev >= 0))
                next_grid[j_prev * width + i].bits |= N_UP;
        }
    }

    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            CAT_PutCell((char *)&next_grid[j * width + i], i, j);
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc < 4 || argc > 5)
    {
        printf("usage: %s file_in file_out num_of_iters <oscillation> (optionally)\n", argv[0]);
        return 1;
    }

    long itersNumber = strtol(argv[3], NULL, 10);

    if (argc == 5 && strcmp(argv[4], "oscillation") == 0)
    {
        is_oscillation = 1;
        printf("Oscillation!!!!!!!!!!!\n");
    }

    printf("Loading: %s\n", argv[1]);
    if (CAT_InitSimulator(argv[1]) != 0)
    {
        printf("Error reading input\n");
        return 1;
    }

    CAT_Index size = CAT_GetArraySize();
    int I = size.i;
    int J = size.j;
    printf("Grid: %dx%d, Iters: %ld\n", I, J, itersNumber);

    cellBody *temp_grid = (cellBody *)malloc(I * J * sizeof(cellBody));
    if (!temp_grid)
    {
        printf("Memory allocation failed\n");
        return 1;
    }

    clock_t t1 = clock();

    for (int i = 0; i < itersNumber; i++)
    {
        if (CAT_Iterate(collision))
        {
            printf("Error at iteration %d\n", i);
            break;
        }

        propagation_step(I, J, temp_grid);
        // CAT_Iterate(propagation);
    }

    clock_t t2 = clock();
    double tDiff = (double)(t2 - t1) / CLOCKS_PER_SEC;
    printf("Time = %g s\n", tDiff);

    free(temp_grid);

    printf("Saving: %s\n", argv[2]);
    if (CAT_FinalizeSimulator(argv[2]) != 0)
    {
        printf("Error writing output\n");
        return 1;
    }
    is_oscillation = 0;

    return 0;
}
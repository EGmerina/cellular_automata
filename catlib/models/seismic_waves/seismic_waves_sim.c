#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "catlib.h"
#include "seismic_waves.h"

int collision(void *n)
{
    cellBody *cell = (cellBody *)n;
    uint8_t state = cell->bits;
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

    cell->bits = new_state;
    return 0;
}

// === ЛОГИКА ПЕРЕМЕЩЕНИЯ (PROPAGATION) ===
// Используем двойную буферизацию, чтобы избежать гонок данных
void propagation_step(int width, int height)
{
    // Временный массив для следующего шага
    // row-major order: index = j * width + i
    uint8_t *next_grid = (uint8_t *)calloc(width * height, sizeof(uint8_t));
    if (!next_grid)
        exit(1);

    cellBody cell;

    // 1. Чтение и разлет частиц
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            CAT_GetCell((char *)&cell, i, j);
            uint8_t b = cell.bits;
            if (b == 0)
                continue;

            // Расчет индексов соседей с периодическими граничными условиями (Тор)
            // Или жесткими стенками (здесь сделаем жесткие стенки - поглощение)

            int i_next = i + 1;
            int i_prev = i - 1;
            int j_next = j + 1;
            int j_prev = j - 1;

            // POSITIVE PARTICLES
            if ((b & P_RIGHT) && (i_next < width))
                next_grid[j * width + i_next] |= P_RIGHT;
            if ((b & P_LEFT) && (i_prev >= 0))
                next_grid[j * width + i_prev] |= P_LEFT;
            if ((b & P_DOWN) && (j_next < height))
                next_grid[j_next * width + i] |= P_DOWN;
            if ((b & P_UP) && (j_prev >= 0))
                next_grid[j_prev * width + i] |= P_UP;

            // NEGATIVE PARTICLES
            if ((b & N_RIGHT) && (i_next < width))
                next_grid[j * width + i_next] |= N_RIGHT;
            if ((b & N_LEFT) && (i_prev >= 0))
                next_grid[j * width + i_prev] |= N_LEFT;
            if ((b & N_DOWN) && (j_next < height))
                next_grid[j_next * width + i] |= N_DOWN;
            if ((b & N_UP) && (j_prev >= 0))
                next_grid[j_prev * width + i] |= N_UP;
        }
    }

    // 2. Запись обратно в CAT
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            cell.bits = next_grid[j * width + i];
            CAT_PutCell((char *)&cell, i, j);
        }
    }

    free(next_grid);
}

int propagation(void *n)
{
    cellBody *cell = (cellBody *)n;

    CAT_Index size = CAT_GetArraySize();
    int W = size.i;
    // int H = size.j; // Можно использовать для проверки границ

    uint8_t new_bits = 0;

    // --- ЛОГИКА "PULL" (ТЯНУТЬ) ---
    // Мы находимся в ячейке 'cell'. Смотрим на соседей и забираем прилетающие частицы.

    // 1. Частица P_RIGHT (летит ВПРАВО)
    // Она должна прийти от соседа СЛЕВА (смещение -1)
    // Проверка (cell - 1) предполагает, что мы не на левой границе массива!
    // *Примечание: Без проверки границ i>0 это опасно, но делаем "как в примере"*
    if ((cell + 2)->bits & P_RIGHT)
    {
        new_bits |= P_RIGHT;
    }
    // Аналогично для отрицательной частицы
    if ((cell + 2)->bits & N_RIGHT)
    {
        new_bits |= N_RIGHT;
    }

    // 2. Частица P_LEFT (летит ВЛЕВО)
    // Приходит от соседа СПРАВА (смещение +1)
    if ((cell + 1)->bits & P_LEFT)
    {
        new_bits |= P_LEFT;
    }
    if ((cell + 1)->bits & N_LEFT)
    {
        new_bits |= N_LEFT;
    }

    // 3. Частица P_UP (летит ВВЕРХ, то есть уменьшает индекс j)
    // Приходит от соседа СНИЗУ (смещение +W, так как j растет вниз)
    if ((cell + 3)->bits & P_UP)
    {
        new_bits |= P_UP;
    }
    if ((cell + 3)->bits & N_UP)
    {
        new_bits |= N_UP;
    }

    // 4. Частица P_DOWN (летит ВНИЗ, увеличивает индекс j)
    // Приходит от соседа СВЕРХУ (смещение -W)
    if ((cell + 4)->bits & P_DOWN)
    {
        new_bits |= P_DOWN;
    }
    if ((cell + 4)->bits & N_DOWN)
    {
        new_bits |= N_DOWN;
    }

    // Записываем новое состояние
    cell->bits = new_bits;

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("usage: %s file_in file_out num_of_iters\n", argv[0]);
        return 1;
    }

    long itersNumber = strtol(argv[3], NULL, 10);

    printf("Loading: %s\n", argv[1]);
    if (CAT_InitSimulator(argv[1]) != 0)
    {
        printf("Error: can't read input file\n");
        return 1;
    }

    CAT_Index size = CAT_GetArraySize();
    int I = size.i;
    int J = size.j;

    printf("Grid: %dx%d, Iters: %ld\n", I, J, itersNumber);

    clock_t t1 = clock();

    for (int i = 0; i < itersNumber; i++)
    {
        int flag = CAT_Iterate(collision);
        if (flag)
        {
            printf("\nError: %d, iteration: %d\n", flag, i);
            return 1;
        }

        // flag = CAT_Iterate(propagation);
        // if (flag)
        // {
        //     printf("\nError: %d, iteration: %d\n", flag, i);
        //     return 1;
        // }
        propagation_step(I, J);
    }

    clock_t t2 = clock();
    double tDiff = (double)(t2 - t1) / CLOCKS_PER_SEC;
    printf("\nTime = %g s\n", tDiff);

    printf("Saving: %s\n", argv[2]);
    if (CAT_FinalizeSimulator(argv[2]) != 0)
    {
        printf("Error writing output\n");
        return 1;
    }

    return 0;
}
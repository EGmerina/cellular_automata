#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "catlib.h"
#include "seismic_waves.h"

#define PROGRESS_BAR 100

// === ЛОГИКА СТОЛКНОВЕНИЙ (HPP MODEL) ===
// HPP правило: если частицы летят навстречу друг другу (Head-On) и нет боковых,
// они поворачивают на 90 градусов.
int collision(void *n)
{
    cellBody *cell = (cellBody*) n;
    uint8_t state = cell->bits;
    uint8_t new_state = state;

    // --- Обработка положительных частиц (биты 0-3) ---
    uint8_t p = state & P_MASK;
    // Проверка на Head-On столкновение по горизонтали: Right(1) + Left(4) = 5
    // И отсутствие вертикальных частиц (чтобы не было тройных столкновений)
    if (p == (P_RIGHT | P_LEFT)) {
        new_state &= ~P_MASK;        // Очищаем pos биты
        new_state |= (P_UP | P_DOWN); // Поворачиваем в Up/Down
    }
    // Проверка на Head-On по вертикали: Up(2) + Down(8) = 10
    else if (p == (P_UP | P_DOWN)) {
        new_state &= ~P_MASK;
        new_state |= (P_RIGHT | P_LEFT);
    }

    // --- Обработка отрицательных частиц (биты 4-7) ---
    uint8_t n_part = state & N_MASK;
    // Right(10h) + Left(40h) = 50h
    if (n_part == (N_RIGHT | N_LEFT)) {
        new_state &= ~N_MASK;
        new_state |= (N_UP | N_DOWN);
    }
    // Up(20h) + Down(80h) = A0h (160 dec)
    else if (n_part == (N_UP | N_DOWN)) {
        new_state &= ~N_MASK;
        new_state |= (N_RIGHT | N_LEFT);
    }

    // Записываем результат обратно
    cell->bits = new_state;
    return 0;
}

// === ЛОГИКА ПЕРЕМЕЩЕНИЯ (PROPAGATION) ===
// Используем двойную буферизацию, чтобы избежать гонок данных
void propagation_step(int width, int height) {
    // Временный массив для следующего шага
    // row-major order: index = j * width + i
    uint8_t *next_grid = (uint8_t*)calloc(width * height, sizeof(uint8_t));
    if (!next_grid) exit(1);

    cellBody cell;

    // 1. Чтение и разлет частиц
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            CAT_GetCell((char*)&cell, i, j);
            uint8_t b = cell.bits;
            if (b == 0) continue;

            // Расчет индексов соседей с периодическими граничными условиями (Тор)
            // Или жесткими стенками (здесь сделаем жесткие стенки - поглощение)
            
            int i_next = i + 1;
            int i_prev = i - 1;
            int j_next = j + 1;
            int j_prev = j - 1;

            // POSITIVE PARTICLES
            if ((b & P_RIGHT) && (i_next < width))  next_grid[j * width + i_next] |= P_RIGHT;
            if ((b & P_LEFT)  && (i_prev >= 0))     next_grid[j * width + i_prev] |= P_LEFT;
            if ((b & P_DOWN)  && (j_next < height)) next_grid[j_next * width + i] |= P_DOWN;
            if ((b & P_UP)    && (j_prev >= 0))     next_grid[j_prev * width + i] |= P_UP;

            // NEGATIVE PARTICLES
            if ((b & N_RIGHT) && (i_next < width))  next_grid[j * width + i_next] |= N_RIGHT;
            if ((b & N_LEFT)  && (i_prev >= 0))     next_grid[j * width + i_prev] |= N_LEFT;
            if ((b & N_DOWN)  && (j_next < height)) next_grid[j_next * width + i] |= N_DOWN;
            if ((b & N_UP)    && (j_prev >= 0))     next_grid[j_prev * width + i] |= N_UP;
        }
    }

    // 2. Запись обратно в CAT
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            cell.bits = next_grid[j * width + i];
            CAT_PutCell((char*)&cell, i, j);
        }
    }

    free(next_grid);
}

int main(int argc, char *argv[])
{
    if(argc != 4) {
        printf("usage: %s file_in file_out num_of_iters\n", argv[0]);
        return 1;
    }

    long itersNumber = strtol(argv[3], NULL, 10);

    printf("Loading: %s\n", argv[1]);
    if (CAT_InitSimulator(argv[1]) != 0) {
        printf("Error: can't read input file\n");
        return 1;
    }

    // Получаем размеры. 
    // Внимание: используем CAT_GetArraySize, так как она надежнее в вашей версии
    CAT_Index size = CAT_GetArraySize();
    int I = size.i;
    int J = size.j;

    printf("Grid: %dx%d, Iters: %ld\n", I, J, itersNumber);

    clock_t t1 = clock();
    
    // ОСНОВНОЙ ЦИКЛ
    for (int i = 0; i < itersNumber; i++)
    {
        // 1. COLLISION (Локально)
        CAT_Iterate(collision);

        // 2. PROPAGATION (Глобально)
        propagation_step(I, J);

        if (i % 10 == 0) { // Прогресс бар
             // можно добавить вывод
        }
    }

    clock_t t2 = clock();
    double tDiff = (double)(t2 - t1) / CLOCKS_PER_SEC;
    printf("\nTime = %g s\n", tDiff);

    printf("Saving: %s\n", argv[2]);
    if (CAT_FinalizeSimulator(argv[2]) != 0) {
        printf("Error writing output\n");
        return 1;
    }

    return 0;
}
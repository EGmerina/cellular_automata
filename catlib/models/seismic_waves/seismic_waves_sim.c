#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <string.h> 
#include "catlib.h" 
#include "seismic_waves.h"

#define PROGRESS_BAR 100

// ... (функции calculateCollision, applyCollisions и transportParticles оставляем те же) ...
// Я продублирую их ниже для полноты картины, чтобы вы могли скопировать файл целиком.

// --- Логика столкновения (HPP модель) ---
uint8_t calculateCollision(uint8_t cur)
{
    uint8_t new_state = 0;
    uint8_t pos = cur & ALL_POSITIVE;
    uint8_t neg = cur & ALL_NEGATIVE;

    int pos_count = __builtin_popcount((unsigned)pos);
    int neg_count = __builtin_popcount((unsigned)neg);

    // Столкновения положительных частиц
    if (pos_count == 2) {
        if ((pos & (POS_RIGHT | POS_LEFT)) == (POS_RIGHT | POS_LEFT)) {
            new_state |= (POS_UP | POS_DOWN);
        }
        else if ((pos & (POS_UP | POS_DOWN)) == (POS_UP | POS_DOWN)) {
            new_state |= (POS_RIGHT | POS_LEFT);
        }
        else {
            new_state |= pos; 
        }
    } else {
        new_state |= pos;
    }

    // Столкновения отрицательных частиц
    if (neg_count == 2) {
        if ((neg & (NEG_RIGHT | NEG_LEFT)) == (NEG_RIGHT | NEG_LEFT)) {
            new_state |= (NEG_UP | NEG_DOWN);
        }
        else if ((neg & (NEG_UP | NEG_DOWN)) == (NEG_UP | NEG_DOWN)) {
            new_state |= (NEG_RIGHT | NEG_LEFT);
        }
        else {
            new_state |= neg;
        }
    } else {
        new_state |= neg;
    }

    return new_state;
}

void applyCollisions(int width, int height)
{
    cellBody cell;
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            CAT_GetCell((char*)&cell, i, j);
            
            uint8_t oldDir = cell.directions;
            uint8_t newDir = calculateCollision(oldDir);

            if (newDir != oldDir) {
                cell.directions = newDir;
                CAT_PutCell((char*)&cell, i, j);
            }
        }
    }
}

void transportParticles(int width, int height)
{
    int sizeBytes = width * height * sizeof(uint8_t);
    uint8_t *next_grid = malloc(sizeBytes);
    
    if (!next_grid) {
        fprintf(stderr, "Error: malloc failed in transportParticles\n");
        exit(1);
    }
    memset(next_grid, 0, sizeBytes);

    cellBody cell;

    // Шаг 1: Читаем и переносим во временный буфер
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            CAT_GetCell((char*)&cell, i, j);
            uint8_t dir = cell.directions;
            
            if (dir == 0) continue;
            
            // Вправо
            if ((dir & POS_RIGHT) && (i < width - 1))
                next_grid[j * width + (i + 1)] |= POS_RIGHT;
            if ((dir & NEG_RIGHT) && (i < width - 1))
                next_grid[j * width + (i + 1)] |= NEG_RIGHT;

            // Влево
            if ((dir & POS_LEFT) && (i > 0))
                next_grid[j * width + (i - 1)] |= POS_LEFT;
            if ((dir & NEG_LEFT) && (i > 0))
                next_grid[j * width + (i - 1)] |= NEG_LEFT;

            // Вниз
            if ((dir & POS_DOWN) && (j < height - 1))
                next_grid[(j + 1) * width + i] |= POS_DOWN;
            if ((dir & NEG_DOWN) && (j < height - 1))
                next_grid[(j + 1) * width + i] |= NEG_DOWN;

            // Вверх
            if ((dir & POS_UP) && (j > 0))
                next_grid[(j - 1) * width + i] |= POS_UP;
            if ((dir & NEG_UP) && (j > 0))
                next_grid[(j - 1) * width + i] |= NEG_UP;
        }
    }

    // Шаг 2: Записываем обратно
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            cell.directions = next_grid[j * width + i];
            CAT_PutCell((char*)&cell, i, j);
        }
    }

    free(next_grid);
}

// === ИЗМЕНЕННАЯ ФУНКЦИЯ MAIN ===
int main(int argc, char *argv[])
{
    if (argc != 4) {
        printf("usage: %s file_in file_out num_of_iters\n", argv[0]);
        return 1;
    }

    long itersNumber = strtol(argv[3], NULL, 10);

    printf("Loading input file: %s\n", argv[1]);
    if (CAT_InitSimulator(argv[1]) != 0) {
        printf("Error initializing simulator\n");
        return 1;
    }

    // ИСПРАВЛЕНИЕ: Используем CAT_GetArraySize вместо CAT_GetMaxI
    CAT_Index size = CAT_GetArraySize();
    int width = size.i;
    int height = size.j;

    printf("Grid Size: %dx%d. Starting %ld iterations...\n", width, height, itersNumber);

    clock_t t1 = clock();

    for (int iter = 0; iter < itersNumber; iter++)
    {
        if (itersNumber > PROGRESS_BAR && iter % (itersNumber / PROGRESS_BAR) == 0) {
            printf(".");
            fflush(stdout);
        }

        applyCollisions(width, height);
        transportParticles(width, height);
    }

    clock_t t2 = clock();
    printf("\nTime = %g s\n", (double)(t2 - t1) / CLOCKS_PER_SEC);

    printf("Saving output file: %s\n", argv[2]);
    if (CAT_FinalizeSimulator(argv[2]) != 0) {
        printf("Error writing output\n");
        return 1;
    }

    return 0;
}
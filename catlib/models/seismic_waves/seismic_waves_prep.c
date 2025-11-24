#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h> // нужен для проверки результата (fabs)
#include "catlib.h"
#include "seismic_waves.h"

const char fileName[] = "initial_state.dat";

// Вероятность заполнения фона (в процентах)
#define BACKGROUND_DENSITY 20 

int main(int argc, char *argv[])
{
    const int globalSize = 0;
    const double Kl = 1.0; // cellsPerMeter
    
    // 1. Подготовка координат (размеров)
    CAT_Coord coordMax;
    coordMax.x = 512.0; // Желаемый размер по X
    coordMax.y = 512.0; // Желаемый размер по Y
    // Если есть Z и T, можно обнулить, если структура это требует, 
    // но обычно x и y достаточно для 2D.

    // 2. Инициализация (Согласно вашей сигнатуре)
    // CAT_InitPreprocessor(int cellSize, int globalSize, double cellsPerMeter, CAT_Coord coord)
    CAT_Coord realCoord = CAT_InitPreprocessor(cellSize, globalSize, Kl, coordMax);

    // 3. Проверка на ошибку
    // Так как возвращается структура, проверяем, не нулевые ли размеры
    if (realCoord.x * realCoord.y == 0.0)
    {
        printf("Error: can't initialize preprocessor (returned 0 size)\n");
        return 1;
    }

    // Получаем целочисленные размеры сетки
    CAT_Index indexMax = CAT_GetArraySize();
    int I = indexMax.i;
    int J = indexMax.j;

    printf("Grid init: %.0f x %.0f -> Integer grid: %d x %d\n", 
           realCoord.x, realCoord.y, I, J);

    srand(time(NULL));

    cellBody cell;
    int center_i = I / 2;
    int center_j = J / 2;
    int radius = I / 20;

    printf("Generating field with background density %d%%\n", BACKGROUND_DENSITY);

    for (int i = 0; i < I; i++) {
        for (int j = 0; j < J; j++) {
            cell.bits = 0;

            // --- 1. ФОНОВЫЙ ШУМ (Noise) ---
            if ((rand() % 100) < BACKGROUND_DENSITY) cell.bits |= P_RIGHT;
            if ((rand() % 100) < BACKGROUND_DENSITY) cell.bits |= P_UP;
            if ((rand() % 100) < BACKGROUND_DENSITY) cell.bits |= P_LEFT;
            if ((rand() % 100) < BACKGROUND_DENSITY) cell.bits |= P_DOWN;

            if ((rand() % 100) < BACKGROUND_DENSITY) cell.bits |= N_RIGHT;
            if ((rand() % 100) < BACKGROUND_DENSITY) cell.bits |= N_UP;
            if ((rand() % 100) < BACKGROUND_DENSITY) cell.bits |= N_LEFT;
            if ((rand() % 100) < BACKGROUND_DENSITY) cell.bits |= N_DOWN;

            // --- 2. ИСТОЧНИК В ЦЕНТРЕ ---
            int dx = i - center_i;
            int dy = j - center_j;
            if (dx*dx + dy*dy <= radius*radius) {
                // Добавляем P-волну (давление)
                cell.bits |= (P_RIGHT | P_UP | P_LEFT | P_DOWN);
                // Убираем отрицательные частицы для контраста
                cell.bits &= ~N_MASK;
            }

            CAT_PutCell((char*)&cell, i, j);
        }
    }

    if (CAT_FinalizePreprocessor((char *)fileName) != 0) {
        printf("Error: can't write output file: %s\n", fileName);
        return 1;
    }

    printf("Success! Initial state created: %s\n", fileName);
    return 0;
}
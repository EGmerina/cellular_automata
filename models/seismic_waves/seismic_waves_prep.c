#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "seismic_waves.h"

const char inputFileName[] = "initial_state.dat";

// Создание точечного источника в центре
void createPointSource(int center_i, int center_j, int radius)
{
    cellBody cell;
    
    for (int i = center_i - radius; i <= center_i + radius; i++) {
        for (int j = center_j - radius; j <= center_j + radius; j++) {
            // Проверяем границы
            if (i < 0 || j < 0) continue;
            
            CAT_GetCell(&cell, i, j);
            
            // Внутри источника - только положительные частицы
            if ((i-center_i)*(i-center_i) + (j-center_j)*(j-center_j) <= radius*radius) {
                cell.directions = ALL_POSITIVE; // 4 положительные частицы
            } else {
                // Вне источника - сбалансированная среда
                cell.directions = ALL_POSITIVE | ALL_NEGATIVE; // 4+4 частицы
            }
            
            CAT_PutCell(&cell, i, j);
        }
    }
}

// Создание слоистой модели Земли
void createEarthModel(int width, int height, int layer_thickness)
{
    cellBody cell;
    
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            // Верхний слой (воздух) - свободная граница
            if (j < layer_thickness) {
                // Плотная среда для демонстрации
                cell.directions = ALL_POSITIVE | ALL_NEGATIVE;
            } else {
                // Основная среда
                cell.directions = ALL_POSITIVE | ALL_NEGATIVE;
            }
            CAT_PutCell(&cell, i, j);
        }
    }
}

int main()
{
    const int globalSize = 0;
    const double Kl = 1.0;
    int width, height;
    CAT_Coord coordMax;

    cellBody *cell;
    cell = malloc(cellSize);
    if (cell == NULL)
        return -1;

    coordMax.x = 1023;
    coordMax.y = 1023;

    coordMax = CAT_InitPreprocessor(cellSize, globalSize, Kl, coordMax);
    if (coordMax.x * coordMax.y == 0.0)
    {
        printf("Error: can't initialize preprocessor\n");
        return 1;
    }
    
    CAT_Index indexMax = CAT_GetArraySize();
    width = indexMax.i;
    height = indexMax.j;
    printf("Grid size: %d x %d\n", width, height);

    // Инициализация сбалансированной среды
    cell->directions = ALL_POSITIVE | ALL_NEGATIVE; // нейтральное состояние
    for (int i = 0; i < width; i++)
        for (int j = 0; j < height; j++)
            CAT_PutCell(cell, i, j);

    // Создание точечного источника в центре
    createPointSource(width/2, height/2, 32);
    
    // Альтернативно: создание слоистой модели
    // createEarthModel(width, height, height/4);

    int flag = CAT_FinalizePreprocessor((char *)inputFileName);
    if (flag)
    {
        printf("Error: can't write output file: %s\n", inputFileName);
        return 1;
    }

    printf("Initial state created successfully\n");
    free(cell);
    return 0;
}
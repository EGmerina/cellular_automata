#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "catlib.h"
#include "seismic_waves.h"

int main(int argc, char *argv[])
{
    const int globalSize = 0;
    const double Kl = 1.0; // cellsPerMeter
    
    // В этой версии библиотеки нужно передавать структуру CAT_Coord для задания размеров
    CAT_Coord coordMax;
    coordMax.x = 400.0; // Размер по X
    coordMax.y = 400.0; // Размер по Y

    // Вызов функции согласно вашей ошибке
    CAT_Coord realCoord = CAT_InitPreprocessor(cellSize, globalSize, Kl, coordMax);

    // Проверка на ошибку (если вернулись нули)
    if (realCoord.x * realCoord.y == 0.0) {
        printf("Error: can't initialize preprocessor\n");
        return 1;
    }

    CAT_Index indexMax = CAT_GetArraySize();
    printf("Grid initialized: %.0f x %.0f (Cells: %d x %d)\n", 
           realCoord.x, realCoord.y, indexMax.i, indexMax.j);

    cellBody cell;

    // Параметры среды
    double c1 = 0.5; 
    double c2 = 0.8; 
    double alpha1 = c1 * c1;
    double alpha2 = c2 * c2;
    
    int interface_y = indexMax.j / 2;

    for (int i = 0; i < indexMax.i; i++) {
        for (int j = 0; j < indexMax.j; j++) {
            cell.u = 0.0;
            cell.u_prev = 0.0;
            cell.damping = 0.999; 

            // Свойства слоев
            if (j < interface_y) {
                cell.velocity_sqr = alpha1;
            } else {
                cell.velocity_sqr = alpha2;
            }

            // Источник волн
            int source_i = indexMax.i / 2;
            int source_j = indexMax.j / 3;
            double dist_sq = (double)((i - source_i)*(i - source_i) + (j - source_j)*(j - source_j));
            
            if (dist_sq < 100.0) {
                cell.u = 10.0 * exp(-dist_sq / 20.0);
                cell.u_prev = cell.u; 
            }

            CAT_PutCell((char*)&cell, i, j);
        }
    }

    char *outputFile = "initial_state.dat";
    if (CAT_FinalizePreprocessor(outputFile) != 0) {
        printf("Error writing output file: %s\n", outputFile);
        return 1;
    }

    printf("Success! Initial state created: %s\n", outputFile);
    return 0;
}
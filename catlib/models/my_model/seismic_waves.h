#ifndef SEISMIC_WAVE_H
#define SEISMIC_WAVE_H

#include <stdint.h>

typedef struct
{
    uint32_t velocity; // скорость распространения в среде (м\с)
    int compression; //положительное или отрицательное(растяжение)
    int time_iteration;
} cellBody;

const int cellSize = sizeof(cellBody);


#endif
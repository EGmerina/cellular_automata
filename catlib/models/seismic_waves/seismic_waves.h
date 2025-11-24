#ifndef SEISMIC_PWAVE_H
#define SEISMIC_PWAVE_H

#include <stdint.h>

typedef struct {
    uint8_t bits; 
    // биты 0-3: положительные частицы 
    // биты 4-7: отрицательные частицы
} cellBody;

const int cellSize = sizeof(cellBody);

#define P_RIGHT 0x01
#define P_UP    0x02
#define P_LEFT  0x04
#define P_DOWN  0x08

#define N_RIGHT 0x10
#define N_UP    0x20
#define N_LEFT  0x40
#define N_DOWN  0x80

#define P_MASK  0x0F
#define N_MASK  0xF0

#endif
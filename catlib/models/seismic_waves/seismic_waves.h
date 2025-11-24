#ifndef SEISMIC_PWAVE_H
#define SEISMIC_PWAVE_H

#include <stdint.h>
#include "../../src/catlib.h"

// Частица с направлением и знаком (амплитудой)
typedef struct {
    uint8_t directions; // битовая маска: 0-3 положительные, 4-7 отрицательные
} cellBody;

const int cellSize = sizeof(cellBody);

// 0-3 биты: положительные частицы (→, ↑, ←, ↓)
// 4-7 биты: отрицательные частицы (→, ↑, ←, ↓)
#define POS_RIGHT 0x01
#define POS_UP    0x02
#define POS_LEFT  0x04
#define POS_DOWN  0x08
#define NEG_RIGHT 0x10
#define NEG_UP    0x20
#define NEG_LEFT  0x40
#define NEG_DOWN  0x80

#define ALL_POSITIVE 0x0F
#define ALL_NEGATIVE 0xF0
#define ALL_PARTICLES 0xFF

extern const int cellSize;

#endif
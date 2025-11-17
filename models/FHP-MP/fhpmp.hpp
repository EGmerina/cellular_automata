#include <stdint.h>
#include <stdio.h>
extern "C" {
#include "catlib.h"
}

typedef struct hexCell {
    uint16_t countCellsDirection0;
    uint16_t countCellsDirection1;
    uint16_t countCellsDirection2;
    uint16_t countCellsDirection3;
    uint16_t countCellsDirection4;
    uint16_t countCellsDirection5;
    uint32_t cellType;
} hexCell;

const int cellSize = sizeof(hexCell);

#define CONVENTIONAL 0
#define INLET 1
#define OUTLET 2
#define WALL 15


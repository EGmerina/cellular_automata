#ifndef CATRAND_H
#define CATRAND_H

#include <stdint.h>
#include <stdlib.h>

extern uint64_t rand40_seed;
extern const double RAND40_MAX;

int CAT_Rand(int RandMax);
void CAT_Srand(unsigned int);

#endif


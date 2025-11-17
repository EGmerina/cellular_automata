#ifdef USE_OMP
#include <omp.h>
#endif

#include "catrand.h"

const uint64_t mask = (1ull << 40) - 1;
const uint64_t coef = 762939453125ull;
const double RAND40_MAX = (double) (1ull << 40);
const double inv_RAND40_MAX = 1.0 / ((double) (1ull << 40));
uint64_t rand40_seed=1;
#pragma omp threadprivate(rand40_seed) // to avoid race condition

uint64_t fast_prng40()
{
    rand40_seed=(rand40_seed * coef) & mask;
    return rand40_seed;
}

/*
 * CAT_Rand - return random number from [?, RandMax]/)???
 * Supposed to be MT-safe. No need for any openmp magic clauses e.g. private(...)
 */
int CAT_Rand(int RandMax)
{
    return (int) ((double) fast_prng40() * inv_RAND40_MAX * RandMax);
}

/*
 * CAT_Srand - ?
 * 
 */
void CAT_Srand(unsigned int seed)
{
	#pragma parallel
	{
		rand40_seed = seed;
#ifdef USE_OMP
		rand40_seed *= omp_get_thread_num() + 1;
#endif
		rand40_seed = fast_prng40();
	}
}


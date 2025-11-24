#pragma once

#define SYNC    101
#define ASYNC   102

#define ALG1    201
#define ALG2    202

#include <stdint.h>
#include "catfile.h"

#ifdef USE_OMP
#include <omp.h>
#define numThreadsMax 1024
#else
#define numThreadsMax 1
#endif

typedef struct CAT_globals_s
{
	caFileHeader *fileHeader;
	char* cellularArray;
#if OPER_MODE == SYNC
	char* cellularArraySwap;
#endif
	char* neighborsThread[numThreadsMax];
	int neighborThreadCnt;
#ifdef USE_OMP
	omp_lock_t* locks;
#endif
} CAT_Globals_t;

enum compressionType {
	UNCOMPRESSED = 0,
	LZMA = 1,
	LZMA_FRAGMENTS = 2,
};

extern caFileHeader *header;

#if OPER_MODE == SYNC
int CAT_Iterate_Sync(int (*cellTransition)(void*));
#endif
#if OPER_MODE == ASYNC
int CAT_Iterate_Async(int (*cellTransition)(void*));
#endif

#ifdef USE_OMP
extern omp_lock_t *locks; // need for topology LockNeighbors/UnlockNeighbors implementation
void CAT_InitLocks(omp_lock_t *m);
void CAT_DestroyLocks(omp_lock_t *m);
void CAT_LockNeighbors(int i, int j, omp_lock_t *);
void CAT_UnlockNeighbors(int i, int j, omp_lock_t *);
#endif


#pragma once

#include <stdint.h>
#include "catlib.h"
#include "catglobals.h"

#ifdef USE_MPI
#include <mpi.h>
extern int CAT_Proc_size;
extern int CAT_Proc_rank;
extern int CAT_Bound_size;
#else
extern const int CAT_Proc_size;
extern const int CAT_Proc_rank;
#endif

enum parallelError {
    GENERIC_ERROR = -20,
    MPI_ERROR = -21,
    INAPPROPRIATE_SIZE = -22,
    ALLOC_ERROR = -23,
    IO_ERROR = -24,
};

int CAT_InitParallelRun();

unsigned int CAT_GetPartSize(uint64_t sizeI, int procRank, int period);

int CAT_FileReadParallel2D(char *filename, CAT_Globals_t* glob, CAT_Index borders, CAT_Index period); //char **caArrayPtr, char **caArrayPtr2);

int CAT_FileSaveParallel2D(char *filename, CAT_Globals_t* glob, CAT_Index period);

#ifdef USE_MPI
int CAT_BoundRecv(char* caArray, MPI_Request* request, MPI_Status* status,
                  unsigned int arraySizeWithBound, unsigned int cellSize, unsigned int shift);

int CAT_BoundSend(char* caArray, MPI_Request* request, MPI_Status* status,
                  unsigned int arraySizeWithBound, unsigned int cellSize, unsigned int shift);
#endif

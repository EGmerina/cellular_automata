#include <assert.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <stdbool.h>
#include <math.h>

#include "catglobals.h"
#include "catfragment.h"
#include "catlib.h"
#include "deps/catrand.h"
#include "catfile.h"
#include "catparallel.h"
#include "topology.h"

const char version[] = "23.12.11";

CAT_Globals_t CAT_Globals = {
	.fileHeader = NULL,
	.cellularArray = NULL,
#if OPER_MODE == SYNC
	.cellularArraySwap = NULL,
#endif

#ifdef USE_OMP
	.locks = NULL,
#endif
};

caFileHeader *header = NULL;

const CAT_Coord err_coord = {.x = -1.0, .y = -1.0, .z = -1.0, .w = -1.0};
const CAT_Index err_index = {.i = -1, .j = -1, .k = -1, .l = -1};

void fillReserved(uint8_t *reserved, int size, uint8_t value)
{
	for (int i = 0; i < size; i++)
		reserved[i] = value;
}

void headerCopy(uint8_t *param, int size, const char *value)
{
	for (int i = 0; i < size; i++)
		param[i] = value[i];
}

int _CAT_GetNeighborsNumber() {
	return CAT_Topology.neighborsNumber;
}

CAT_Index CAT_GetBorderSize() {
    return CAT_Topology.borderWidth;
}

CAT_Index CAT_GetLatticePeriod() {
    return CAT_Topology.latticePeriod;
}

CAT_Coord CAT_GetCoord(CAT_Index index) {
	if (!header) return err_coord;
	return CAT_Topology.GetCoord(index);
}

CAT_Coord CAT_GetMetricSize() {
	if (!header) return err_coord;
	return CAT_Topology.GetMetricSize();
}

CAT_Index CAT_GetIndex(CAT_Coord coord) {
	if (!header) return err_index;
	return CAT_Topology.GetIndex(coord);
}

CAT_Index CAT_GetArraySize() {
	if (!header) return err_index;
	return CAT_Topology.GetArraySize();
}

double CAT_SquareDistance(CAT_Index index1, CAT_Index index2) {
	if (!header) return -1.0;
	return CAT_Topology.SquareDistance(index1, index2);
}

double CAT_GetCellsPerMeter() {
	if (!header) return -1.0;
	return header->cellsPerMeter;
}

int CAT_GetIterationsDone() {
	if (!header) return -1;
	return header->iterationsDone;
}

CAT_Coord CAT_InitPreprocessor(int cellSize, int globalSize, double cellsPerMeter, CAT_Coord coord) {
	CAT_PrintEnvironment();
	header = malloc(sizeof(caFileHeader));
	if (header == NULL) {
		coord.x = 0.0;
		coord.y = 0.0;
		coord.z = 0.0;
		coord.w = 0.0;
		return coord;
	}
	header->cellSize		 = cellSize;
	header->globalSize	   = globalSize;
	header->cellsPerMeter  = cellsPerMeter;

	headerCopy(header->caSignature, 2, "CA");
	header->dimension		= CAT_Topology.dimension + '0';
	header->dSignature	   = 'D';
	headerCopy(header->vSignature, 4, "v.20");
	headerCopy(header->version, 8, version);
//	header->version		  = ((uint32_t)'1')+((uint32_t)'.'<<8)+((uint32_t)'0'<<16)+((uint32_t)'0'<<24);
	header->arrayTopology	= TOPOLOGY;
	header->modelType		= 0;
	header->operationMode	= OPER_MODE;
	header->compressionType  = LZMA_FRAGMENTS;

	CAT_Index index = CAT_Topology.GetIndex(coord);
	int dim = CAT_Topology.dimension;
	int period = CAT_Topology.latticePeriod.i;
	int mod = (index.i + 1) % period;
	header->arraySizeI	   = dim < 1 ? 1 : index.i + 1 + (mod > 0) * period - mod;
	period = CAT_Topology.latticePeriod.j;
	mod = (index.j + 1) % period;
	header->arraySizeJ	   = dim < 2 ? 1 : index.j + 1 + (mod > 0) * period - mod;
	period = CAT_Topology.latticePeriod.k;
	mod = (index.k + 1) % period;
	header->arraySizeK	   = dim < 3 ? 1 : index.k + 1 + (mod > 0) * period - mod;
	period = CAT_Topology.latticePeriod.l;
	mod = (index.l + 1) % period;
	header->arraySizeL	   = dim < 4 ? 1 : index.l + 1 + (mod > 0) * period - mod;
	header->iterationsDone = 0;

	fillReserved(header->reserved96, sizeof (header->reserved96), 0);
	fillReserved(header->reserved128, sizeof (header->reserved128), 0);
	fillReserved(header->reserved256, sizeof (header->reserved256), 0);
	fillReserved(header->reserved512, sizeof (header->reserved512), 0);

	CAT_Globals.cellularArray = malloc(globalSize + cellSize * header->arraySizeI * header->arraySizeJ * header->arraySizeK * header->arraySizeL);
	if (CAT_Globals.cellularArray == NULL) {
		coord.x = 0.0;
		coord.y = 0.0;
		coord.z = 0.0;
		coord.w = 0.0;
		return coord;
	}

	return CAT_Topology.GetMetricSize();
}

int CAT_InitPostprocessor(char *filename){
	CAT_PrintEnvironment();
	if (filename == NULL) return -5;
	return CAT_FileRead(filename, &header, &(CAT_Globals.cellularArray));
}

#if OPER_MODE == SYNC
void CAT_PutResult2D(void *cellValue, int i, int j)
{
	char *cellValue1;
	cellValue1 = cellValue;
	int cellSize = header->cellSize;
	int cellStart = (i * header->arraySizeJ + j) * cellSize;
	for (int k = 0; k < cellSize; k++)
		CAT_Globals.cellularArraySwap[cellStart + k] = cellValue1[k];
	return;
}

void CAT_PutResult3D(void *cellValue, int i, int j, int k)
{
	char* cellValue1;
    cellValue1 = cellValue;
    int cellSize = header->cellSize;
    int cellStart = (i * header->arraySizeJ * header->arraySizeK + j * header->arraySizeK + k) * cellSize;
    for (int m = 0; m < cellSize; m++)
		CAT_Globals.cellularArraySwap[cellStart + m] = cellValue1[m];
    return;
}
#endif

#if CAT_TOP_DIMENSION == 2
void CAT_PutCell2D(void *cellValue, int i, int j)
{
	char *cellValue1;
	cellValue1 = cellValue;
	int cellSize = header->cellSize;
	int cellStart = (i * header->arraySizeJ + j) * cellSize;
	for (int k = 0; k < cellSize; k++)
		CAT_Globals.cellularArray[cellStart + k] = cellValue1[k];
	return;
}

void CAT_PutCell(void *cellValue, int i, int j) // temporary 2D only
{
	char *cellValue1;
	cellValue1 = cellValue;
	int cellSize = header->cellSize;
	int cellStart = (i * header->arraySizeJ + j) * cellSize;
	for (int k = 0; k < cellSize; k++)
		CAT_Globals.cellularArray[cellStart + k] = cellValue1[k];
	return;
}

void CAT_GetCell2D(void *cellValue, int i, int j)
{
	int cellSize = header->cellSize;
	int cellStart = (i * header->arraySizeJ + j) * cellSize;
	memcpy(cellValue, &(CAT_Globals.cellularArray[cellStart]), cellSize);
	return;
}

void CAT_GetCell(void *cellValue, int i, int j) // temporary 2D only
{
	int cellSize = header->cellSize;
	int cellStart = (i * header->arraySizeJ + j) * cellSize;
	memcpy(cellValue, &(CAT_Globals.cellularArray[cellStart]), cellSize);
	return;
}
#endif

#if CAT_TOP_DIMENSION == 3

void CAT_GetCell3D(void* cellValue, int i, int j, int k)
{
    char* cellValue1;
    cellValue1 = cellValue;
    int cellSize = header->cellSize;
    int cellStart = (i * header->arraySizeJ * header->arraySizeK + j * header->arraySizeK + k) * cellSize;
    for (int m = 0; m < cellSize; m++)
        cellValue1[m] = CAT_Globals.cellularArray[cellStart + m];
    return;
}

void CAT_PutCell3D(void* cellValue, int i, int j, int k)
{
    char* cellValue1;
    cellValue1 = cellValue;
    int cellSize = header->cellSize;
    int cellStart = (i * header->arraySizeJ * header->arraySizeK + j * header->arraySizeK + k) * cellSize;
    for (int m = 0; m < cellSize; m++)
	CAT_Globals.cellularArray[cellStart + m] = cellValue1[m];
    return;
}
#endif

int CAT_FinalizePreprocessor(char *filename)
{
	int flag = CAT_FileSave(filename, &header, &(CAT_Globals.cellularArray));
	free(header);
	free(CAT_Globals.cellularArray);
	return flag;
}

int CAT_FinalizePostprocessor(){
	free(header);
	free(CAT_Globals.cellularArray);
	return 0;
}

void CAT_PrintEnvironment() {
#ifdef USE_MPI
	printf("USE_MPI is ON\n");
	#ifdef PAR_MODE
		#if PAR_MODE == ALG1
			printf("ALG1 is activated\n");
		#endif
		#if PAR_MODE == ALG2
			printf("ALG2 is activated\n");
		#endif
	#endif
#else
	printf("USE_MPI is OFF\n");
#endif

#ifdef USE_OMP
	printf("USE_OMP is ON, %d thread(s)\n", CAT_GetNumThreads());
#else
	printf("USE_OMP is OFF\n");
#endif

#ifdef TOPOLOGY
	printf("TOPOLOGY = %s, %dD, %d neighbors\n", CAT_Topology.name, CAT_TOP_DIMENSION, CAT_TOP_NEIGHBORS_NUMBER);
	char lpj[] = "  -", lpk[] = "  -", lpl[] = "  -", bwj[] = "  -", bwk[] = "  -", bwl[] = "  -";
	switch(CAT_TOP_DIMENSION) {
		case 4:
		sprintf(lpl, "%3d", CAT_Topology.latticePeriod.l);
		sprintf(bwl, "%3d", CAT_Topology.borderWidth.l);
		case 3:
		sprintf(lpk, "%3d", CAT_Topology.latticePeriod.k);
		sprintf(bwk, "%3d", CAT_Topology.borderWidth.k);
		case 2:
		sprintf(lpj, "%3d", CAT_Topology.latticePeriod.j);
		sprintf(bwj, "%3d", CAT_Topology.borderWidth.j);
	}
	printf("╔════════════════╤═════╤═════╤═════╤═════╗\n");
	printf("║                │  i  │  j  │  k  │  l  ║\n");
	printf("╟────────────────┼─────┼─────┼─────┼─────╢\n");
	printf("║ lattice period │%3d  │%s  │%s  │%s  ║\n", CAT_Topology.latticePeriod.i, lpj, lpk, lpl);
	printf("╟────────────────┼─────┼─────┼─────┼─────╢\n");
	printf("║ border width   │%3d  │%s  │%s  │%s  ║\n", CAT_Topology.borderWidth.i, bwj, bwk, bwl);
	printf("╚════════════════╧═════╧═════╧═════╧═════╝\n");
#else
	printf("TOPOLOGY is missed\n");
#endif

#ifdef OPER_MODE
	#if OPER_MODE == SYNC
		printf("SYNC operation mode is activated\n");
	#endif
	#if OPER_MODE == ASYNC
		printf("ASYNC operation mode is activated\n");
	#endif
	#if (OPER_MODE != SYNC) && (OPER_MODE != ASYNC)
		printf("UNKNOWN operation mode is activated\n");
	#endif
#else
	printf("No OPER_MODE is activated\n");
#endif

#ifndef USE_MPI
	#ifdef PAR_MODE
		#if PAR_MODE == ALG1
			printf("ALG1 is ignored\n");
		#endif
		#if PAR_MODE == ALG2
			printf("ALG2 is ignored\n");
		#endif
		#if (PAR_MODE != ALG1) && (PAR_MODE != ALG2)
			printf("Unknown PAR_MODE is ignored\n");
		#endif
	#endif
#else
	printf("USE_MPI is OFF\n");
#endif
	fflush(stdout);
}

int CAT_InitSimulator(char* filename) {
	CAT_PrintEnvironment();
	CAT_Srand(clock());
	if (filename == NULL) return -5;
#ifdef USE_MPI
    CAT_InitParallelRun();
    int flag = CAT_FileReadParallel2D(filename, &CAT_Globals, CAT_GetBorderSize(), CAT_GetLatticePeriod());
	header = CAT_Globals.fileHeader;
    unsigned int cellSize = header->cellSize;
#else
	int flag = CAT_FileRead(filename, &header, &(CAT_Globals.cellularArray));
	if (flag != 0) return flag;
	unsigned int cellSize = header->cellSize;
	unsigned int arraySize = header->arraySizeI *
							 header->arraySizeJ *
							 header->arraySizeK *
							 header->arraySizeL;

	if (header->arrayTopology != TOPOLOGY) {
		return -15;
	}

#if OPER_MODE == SYNC
	CAT_Globals.cellularArraySwap = malloc(cellSize * arraySize);
	if (CAT_Globals.cellularArraySwap == NULL) return -1;
#endif
#endif

#ifdef USE_OMP
	CAT_Globals.locks = malloc(header->arraySizeI * header->arraySizeJ * sizeof(CAT_Globals.locks[0]));
	CAT_InitLocks(CAT_Globals.locks);
#endif

	int neighborsNumber = _CAT_GetNeighborsNumber();
	int neighborsThreadSize = (neighborsNumber + 1) * cellSize + 63;
	neighborsThreadSize -= neighborsThreadSize % 64;
	for(int i = 0; i < numThreadsMax; i++)
	{
		CAT_Globals.neighborsThread[i] = (char*)malloc(neighborsThreadSize);
		if (CAT_Globals.neighborsThread[i] == NULL)
			return -10;
		
		CAT_Globals.neighborThreadCnt++;
	}
	return 0;
}

int CAT_GetNumThreads(void)
{
#ifdef USE_OMP
	int numThreads;
	#pragma omp parallel
		numThreads = omp_get_num_threads();
	return numThreads;
#else
	return 1;
#endif
}

int CAT_SetNumThreads(int numThreads)
{
#ifdef USE_OMP
	if(numThreads > 0 && numThreads <= numThreadsMax)
	{
		omp_set_num_threads(numThreads);
		return 0;
	}
#else
	if(numThreads == 1)
		return 0;
#endif
	return -1;
}

#ifdef USE_OMP
void CAT_InitLocks(omp_lock_t *m) {
	int I = header->arraySizeI;
	int J = header->arraySizeJ;
	for (int i = 0; i < I*J; i++) {
		omp_init_lock(&m[i]);
	}
}

void CAT_DestroyLocks(omp_lock_t *m) {
	int I = header->arraySizeI;
	int J = header->arraySizeJ;
	for (int i = 0; i < I*J; i++) {
		omp_destroy_lock(&m[i]);
	}
}

void _CAT_LockNeighbors(CAT_Index index, omp_lock_t *locks) {
	if (CAT_Topology.LockNeighbors == NULL) printf("null\n");
	CAT_Topology.LockNeighbors(index, locks);
}

void _CAT_UnlockNeighbors(CAT_Index index, omp_lock_t *locks) {
	if (CAT_Topology.UnlockNeighbors == NULL) printf("null\n");
	CAT_Topology.UnlockNeighbors(index, locks);
}
#endif

#if OPER_MODE == SYNC
int CAT_Iterate_Part(int downBoundI, int upBoundI, int downBoundJ, int upBoundJ, int (*cellTransition)(void*),
                     int* errSum, void* neighbors) {
    int CAT_Bound_size = CAT_GetBorderSize().i;
    int flag = 0;
#pragma omp for
    for (int i = downBoundI; i < upBoundI; i++) {
        for (int j = downBoundJ; j < upBoundJ; j++) {
        	CAT_Index index;
        	index.i = i + CAT_Bound_size;
        	index.j = j;
        	CAT_Topology.GetNeighbors(index, neighbors);
            flag = (*cellTransition)(neighbors);	// TODO: cellTransition errors
            *errSum -= !flag;

            CAT_PutResult2D(neighbors, i + CAT_Bound_size, j);
        }
    }
}

#ifdef USE_MPI
int CAT_Iterate_Sync_Parallel(int (*cellTransition)(void*)) {
    int I = CAT_GetPartSize(header->arraySizeI, CAT_Proc_rank, CAT_GetLatticePeriod().i);
    int J = header->arraySizeJ;
    int partArraySize = I * J;
    unsigned int arraySizeWithBound = partArraySize + 2 * CAT_Bound_size * J;
    int cellSize = (header)->cellSize;
    int flag = 0;
    int errSum = I * J;
    int CAT_Bound_size = CAT_GetBorderSize().i;
    char *ca_temp = CAT_Globals.cellularArray;
    MPI_Request request[4];
    MPI_Status status[4];
#ifdef USE_OMP
    if(CAT_GetNumThreads() > numThreadsMax)
        return -1;
#pragma omp parallel reduction(+:errSum)
#endif
    {
        CAT_BoundRecv(CAT_Globals.cellularArray, request, status, arraySizeWithBound, cellSize, CAT_Bound_size * (header)->arraySizeJ);
        void *neighbors;
#ifdef USE_OMP
        neighbors = CAT_Globals.neighborsThread[omp_get_thread_num()];
#else
        neighbors = CAT_Globals.neighborsThread[0];
#endif

		// Iterate the I bounds first
        CAT_Iterate_Part(0, CAT_Bound_size, 0, J, cellTransition,
                         &errSum, neighbors);
        CAT_Iterate_Part(I - CAT_Bound_size, I, 0, J, cellTransition,
                         &errSum, neighbors);
        CAT_BoundSend(CAT_Globals.cellularArray, request, status, arraySizeWithBound, cellSize, CAT_Bound_size * (header)->arraySizeJ);
		// Then iterate the "inner" part
        CAT_Iterate_Part(CAT_Bound_size, I - CAT_Bound_size, 0, J, cellTransition,
                         &errSum, neighbors);
    }

    CAT_Globals.cellularArray = CAT_Globals.cellularArraySwap;
    CAT_Globals.cellularArraySwap = ca_temp;

    MPI_Waitall(4, request, status);
    fflush(stdout);
    header->iterationsDone++;
    return errSum;
}
#endif
#endif

int CAT_Iterate(int (*cellTransition)(void*)) // Actually it's CAT_Iterate2D
{
#if OPER_MODE == SYNC
#ifdef USE_MPI
            if (CAT_Proc_size == 1) {
                return CAT_Iterate_Sync(cellTransition);
            } else {
                return CAT_Iterate_Sync_Parallel(cellTransition);
            }
#else
            return CAT_Iterate_Sync(cellTransition);
#endif
#endif

#if OPER_MODE == ASYNC
#ifdef USE_MPI
		    if (CAT_Proc_size == 1) {
                return CAT_Iterate_Sync(cellTransition);
            } else {
                return CAT_Iterate_Sync_Parallel(cellTransition);
            }
#else
			return CAT_Iterate_Async(cellTransition);
#endif
#endif
			return 1;
}

#if OPER_MODE == SYNC
int CAT_Iterate_Sync(int (*cellTransition)(void*)) {

	 typedef struct cellBody
    {
        uint8_t Value;
    } cellBody;

    int cellSize = sizeof(cellBody);

    cellBody *cell;
    cell = malloc(cellSize);
    if (cell == NULL)
        return -1;

    cell->Value = 0;

	int flag = 0;
	int I = header->arraySizeI;
	int J = header->arraySizeJ;
	int errSum = I * J;
#if CAT_TOP_DIMENSION == 3
    int K = header->arraySizeK;
    errSum *= K;
#endif
// TODO: return errcode if rank > 1 && !mpiInitialized
	char *ca_temp = CAT_Globals.cellularArray;

#ifdef USE_OMP
	if(CAT_GetNumThreads() > numThreadsMax)
		return -1;
	#pragma omp parallel reduction(+:errSum)
#endif
	{
		void *neighbors;
#ifdef USE_OMP
		neighbors = CAT_Globals.neighborsThread[omp_get_thread_num()];
#else
        neighbors = CAT_Globals.neighborsThread[0];
#endif

		#pragma omp for
#if CAT_TOP_DIMENSION == 2
		for (int i = 0; i < I; i++)
		{
			for (int j = 0; j < J; j++)
			{
				CAT_Index index;
				index.i = i;
				index.j = j;
				CAT_Topology.GetNeighbors(index, neighbors);

				flag = (*cellTransition)(neighbors);
				errSum -= !flag;

				CAT_PutResult2D(neighbors, i, j);
			}
		}
#endif
#if CAT_TOP_DIMENSION == 3
        for (int i = 0; i < I; i++)
                {
                    for (int j = 0; j < J; j++)
                    {
                        for (int k = 0; k < K; k++)
                            {
                                CAT_Index index;
                                index.i = i;
                                index.j = j;
                                index.k = k;
                                CAT_Topology.GetNeighbors(index, neighbors);

                                flag = (*cellTransition)(neighbors);
                                errSum -= !flag;

                                CAT_PutResult3D(neighbors, i, j, k);
                            }
                        }
                    }
#endif
	}
	CAT_Globals.cellularArray = CAT_Globals.cellularArraySwap;
	CAT_Globals.cellularArraySwap = ca_temp;
	(header->iterationsDone)++;
	free(cell);

	return errSum;
}
#endif

#if OPER_MODE == ASYNC
int CAT_Iterate_Async(int (*cellTransition)(void*)){
	int flag = 0;
	int I = header->arraySizeI;
	int J = header->arraySizeJ;
	int errSum = I * J;

#if CAT_TOP_DIMENSION == 3
    int K = header->arraySizeK;
    errSum *= K;
#endif

	int neighborsNumber = _CAT_GetNeighborsNumber();
	unsigned int cellSize = header->cellSize;

#ifdef USE_OMP
	if(CAT_GetNumThreads() > numThreadsMax)
		return -1;
	#pragma omp parallel reduction(+:errSum)
#endif
	{
		char *neighbors = (char*)malloc((neighborsNumber + 1) * cellSize + 63);
		#pragma omp for
		for (int counter = 0; counter < I*J; counter++) {
			CAT_Index index;
			index.i = CAT_Rand(I);
			index.j = CAT_Rand(J);
#ifdef USE_OMP
			_CAT_LockNeighbors(index, CAT_Globals.locks);
#endif
			CAT_Topology.GetNeighbors(index, neighbors);

			flag = (*cellTransition)(neighbors);
			errSum -= !flag;
			CAT_Topology.PutNeighbors(index, neighbors);
#ifdef USE_OMP
			_CAT_UnlockNeighbors(index, CAT_Globals.locks);
#endif
		}
	}
	(header->iterationsDone)++;
	return errSum;
}
#endif

int CAT_FinalizeSimulator(char *filename) {
#ifdef USE_MPI
    int flag = CAT_FileSaveParallel2D(filename, &CAT_Globals, CAT_GetLatticePeriod());
#else
    int flag = CAT_FileSave(filename, &header, &(CAT_Globals.cellularArray));
#endif
    for (int i = 0; i < CAT_Globals.neighborThreadCnt; i++) {
		free(CAT_Globals.neighborsThread[i]);
	}
    free(CAT_Globals.cellularArray);
#if OPER_MODE == SYNC
    free(CAT_Globals.cellularArraySwap);
#endif
    // free(header);
#ifdef USE_OMP
    CAT_DestroyLocks(CAT_Globals.locks);
#endif
#ifdef USE_MPI
    flag = MPI_Finalize();
#endif
    return flag;
}


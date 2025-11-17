#include "../topology.h"

#define PERIOD 1
#define BORDER_SIZE 1

void square8_get_neigh(CAT_Index index, void *n) {
	int sizeI = header->arraySizeI;
	int sizeJ = header->arraySizeJ;
	int cellSize = header->cellSize;
	char *n1;
	n1 = n;

	int i = index.i;
	int j = index.j;

	CAT_GetCell2D(n1, i, j);
	CAT_GetCell2D(n1 + cellSize, (i-1+sizeI)%sizeI, j);
	CAT_GetCell2D(n1 + 2 * cellSize, (i-1+sizeI)%sizeI, (j+1)%sizeJ);
	CAT_GetCell2D(n1 + 3 * cellSize, i, (j+1)%sizeJ);
	CAT_GetCell2D(n1 + 4 * cellSize, (i+1)%sizeI, (j+1)%sizeJ);
	CAT_GetCell2D(n1 + 5 * cellSize, (i+1)%sizeI, j);
	CAT_GetCell2D(n1 + 6 * cellSize, (i+1)%sizeI, (j-1+sizeJ)%sizeJ);
	CAT_GetCell2D(n1 + 7 * cellSize, i, (j-1+sizeJ)%sizeJ);
	CAT_GetCell2D(n1 + 8 * cellSize, (i-1+sizeI)%sizeI, (j-1+sizeJ)%sizeJ);
		
	return;
}

void square8_put_neigh(CAT_Index index, void *n) {
	int sizeI = CAT_GetPartSize(header->arraySizeI,  CAT_Proc_rank, PERIOD);
	int sizeJ = header->arraySizeJ;
	int cellSize = header->cellSize;
	char *n1;
	n1 = n;

	int i = index.i;
	int j = index.j;

	CAT_PutCell2D(n1, i, j);
	CAT_PutCell2D(n1 + cellSize, (i-1+sizeI)%sizeI, j);
	CAT_PutCell2D(n1 + 2 * cellSize, (i-1+sizeI)%sizeI, (j+1)%sizeJ);
	CAT_PutCell2D(n1 + 3 * cellSize, i, (j+1)%sizeJ);
	CAT_PutCell2D(n1 + 4 * cellSize, (i+1)%sizeI, (j+1)%sizeJ);
	CAT_PutCell2D(n1 + 5 * cellSize, (i+1)%sizeI, j);
	CAT_PutCell2D(n1 + 6 * cellSize, (i+1)%sizeI, (j-1+sizeJ)%sizeJ);
	CAT_PutCell2D(n1 + 7 * cellSize, i, (j-1+sizeJ)%sizeJ);
	CAT_PutCell2D(n1 + 8 * cellSize, (i-1+sizeI)%sizeI, (j-1+sizeJ)%sizeJ);

	return;
}

CAT_Coord square8_get_coord(CAT_Index index) {
	CAT_Coord coord;
	coord.x = (double) index.i / header->cellsPerMeter;
	coord.y = (double) index.j / header->cellsPerMeter;
	return coord;
}

CAT_Coord square8_get_metric_size(void) {
	CAT_Coord coord;
	coord.x = (double) (header->arraySizeI - 1) / header->cellsPerMeter;
	coord.y = (double) (header->arraySizeJ - 1) / header->cellsPerMeter;
	return coord;
}

CAT_Index square8_get_index(CAT_Coord coord) {
	CAT_Index index;
	index.i = (int) (coord.x * header->cellsPerMeter + 0.5);
	index.j = (int) (coord.y * header->cellsPerMeter + 0.5);
	return index;
}

CAT_Index square8_get_array_size(void) {
	CAT_Index index;
	index.i = header->arraySizeI;
	index.j = header->arraySizeJ;
	return index;
}

double square8_square_distance(CAT_Index index1, CAT_Index index2) {
	CAT_Coord coord1, coord2;
	coord1 = square8_get_coord(index1);
	coord2 = square8_get_coord(index2);
	double dx = coord2.x - coord1.x;
	double dy = coord2.y - coord1.y;
	return dx * dx + dy * dy;
}

#ifdef USE_OMP
void square8_lock_neigh(CAT_Index index, omp_lock_t *locks) {
	/*int I = header->arraySizeI;
	int J = header->arraySizeJ;
	
	int i = index.i;
	int j = index.j;

	CAT_GetCell(n1, i, j);
	CAT_GetCell(n1 + cellSize, (i-1+sizeI)%sizeI, j);
	CAT_GetCell(n1 + 2 * cellSize, (i-1+sizeI)%sizeI, (j+1)%sizeJ);
	CAT_GetCell(n1 + 3 * cellSize, i, (j+1)%sizeJ);
	CAT_GetCell(n1 + 4 * cellSize, (i+1)%sizeI, (j+1)%sizeJ);
	CAT_GetCell(n1 + 5 * cellSize, (i+1)%sizeI, j);
	CAT_GetCell(n1 + 6 * cellSize, (i+1)%sizeI, (j-1+sizeJ)%sizeJ);
	CAT_GetCell(n1 + 7 * cellSize, i, (j-1+sizeJ)%sizeJ);
	CAT_GetCell(n1 + 8 * cellSize, (i-1+sizeI)%sizeI, (j-1+sizeJ)%sizeJ);*/

	return;
}

void square8_unlock_neigh(CAT_Index index, omp_lock_t *locks) {
	/*int I = header->arraySizeI;
	int J = header->arraySizeJ;
	
	int i = index.i;
	int j = index.j;

	CAT_GetCell(n1, i, j);
	CAT_GetCell(n1 + cellSize, (i-1+sizeI)%sizeI, j);
	CAT_GetCell(n1 + 2 * cellSize, (i-1+sizeI)%sizeI, (j+1)%sizeJ);
	CAT_GetCell(n1 + 3 * cellSize, i, (j+1)%sizeJ);
	CAT_GetCell(n1 + 4 * cellSize, (i+1)%sizeI, (j+1)%sizeJ);
	CAT_GetCell(n1 + 5 * cellSize, (i+1)%sizeI, j);
	CAT_GetCell(n1 + 6 * cellSize, (i+1)%sizeI, (j-1+sizeJ)%sizeJ);
	CAT_GetCell(n1 + 7 * cellSize, i, (j-1+sizeJ)%sizeJ);
	CAT_GetCell(n1 + 8 * cellSize, (i-1+sizeI)%sizeI, (j-1+sizeJ)%sizeJ);*/

	return;
}
#endif

struct topology square8 = {
	.name = "SQUARE8",
	.dimension = CAT_TOP_DIMENSION,
	.neighborsNumber = CAT_TOP_NEIGHBORS_NUMBER,
	.latticePeriod.i = PERIOD,
	.latticePeriod.j = PERIOD,
	.latticePeriod.k = PERIOD,
	.latticePeriod.l = PERIOD,
	.borderWidth.i = BORDER_SIZE,
	.borderWidth.j = BORDER_SIZE,
	.borderWidth.k = BORDER_SIZE,
	.borderWidth.l = BORDER_SIZE,
	.GetCoord = square8_get_coord,
	.GetMetricSize = square8_get_metric_size,
	.GetIndex = square8_get_index,
	.GetArraySize = square8_get_array_size,
	.SquareDistance = square8_square_distance,
#ifdef USE_OMP
	.LockNeighbors = square8_lock_neigh,
	.UnlockNeighbors = square8_unlock_neigh,
#endif
	.GetNeighbors = square8_get_neigh,
	.PutNeighbors = square8_put_neigh
};


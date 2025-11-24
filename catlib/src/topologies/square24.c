#include "../topology.h"

#define PERIOD 1
#define BORDER_SIZE 2

void square24_get_neigh(CAT_Index index, void *n) {
	const int SQUARE24_WINDOW_SIZE = 5;
	int sizeI = header->arraySizeI;
	int sizeJ = header->arraySizeJ;
	int cellSize = header->cellSize;
	char *n1;
	n1 = n;

	int i = index.i;
	int j = index.j;

	CAT_GetCell2D(n1, i, j);
	int shift = SQUARE24_WINDOW_SIZE >> 1;
	int beginI = i - shift;
	int beginJ = j - shift;
	int sumCellSize = cellSize;
	int currIndexI = 0;
	int currIndexJ = 0;
	for (int I = 0; I < SQUARE24_WINDOW_SIZE; ++I) {
		for (int J = 0; J < SQUARE24_WINDOW_SIZE; ++J) {
			currIndexI = (beginI + sizeI + I) % sizeI;
			currIndexJ = (beginJ + sizeJ + J) % sizeJ;
			//skip central cell
			if (currIndexI == i && currIndexJ == j) {
				continue;
			}
			CAT_GetCell2D(n1 + sumCellSize, (beginI + sizeI + I) % sizeI, (beginJ + sizeJ + J) % sizeJ);
			sumCellSize += cellSize;
		}
	}
}

void square24_put_neigh(CAT_Index index, void *n) {
	const int SQUARE24_WINDOW_SIZE = 5;
	int sizeI = CAT_GetPartSize(header->arraySizeI,  CAT_Proc_rank, PERIOD);
	int sizeJ = header->arraySizeJ;
	int cellSize = header->cellSize;
	char *n1;
	n1 = n;

	int i = index.i;
	int j = index.j;

	CAT_PutCell2D(n1, i, j);
	int shift = SQUARE24_WINDOW_SIZE >> 1;
	int beginI = i - shift;
	int beginJ = j - shift;
	int sumCellSize = cellSize;
	int currIndexI = 0;
	int currIndexJ = 0;
	for (int I = 0; I < SQUARE24_WINDOW_SIZE; ++I) {
		for (int J = 0; J < SQUARE24_WINDOW_SIZE; ++J) {
			currIndexI = (beginI + sizeI + I) % sizeI;
			currIndexJ = (beginJ + sizeJ + J) % sizeJ;
			//skip central cell
			if (currIndexI == i && currIndexJ == j) {
				continue;
			}
			CAT_PutCell2D(n1 + sumCellSize, (beginI + sizeI + I) % sizeI, (beginJ + sizeJ + J) % sizeJ);
			sumCellSize += cellSize;
		}
	}
	return;
}

CAT_Coord square24_get_coord(CAT_Index index) {
	CAT_Coord coord;
	coord.x = (double) index.i / header->cellsPerMeter;
	coord.y = (double) index.j / header->cellsPerMeter;
	return coord;
}

CAT_Coord square24_get_metric_size(void) {
	CAT_Coord coord;
	coord.x = (double) (header->arraySizeI - 1) / header->cellsPerMeter;
	coord.y = (double) (header->arraySizeJ - 1) / header->cellsPerMeter;
	return coord;
}

CAT_Index square24_get_index(CAT_Coord coord) {
	CAT_Index index;
	index.i = (int) (coord.x * header->cellsPerMeter + 0.5);
	index.j = (int) (coord.y * header->cellsPerMeter + 0.5);
	return index;
}

CAT_Index square24_get_array_size(void) {
	CAT_Index index;
	index.i = header->arraySizeI;
	index.j = header->arraySizeJ;
	return index;
}

double square24_square_distance(CAT_Index index1, CAT_Index index2) {
	CAT_Coord coord1, coord2;
	coord1 = square24_get_coord(index1);
	coord2 = square24_get_coord(index2);
	double dx = coord2.x - coord1.x;
	double dy = coord2.y - coord1.y;
	return dx * dx + dy * dy;
}

#ifdef USE_OMP
void square24_lock_neigh(CAT_Index index, omp_lock_t *locks) {
	int I = header->arraySizeI;
	int J = header->arraySizeJ;
	
	int i = index.i;
	int j = index.j;

	omp_set_lock(&(locks[(i-2+I)%I * J + (j-2+J)%J]));
	omp_set_lock(&(locks[(i-2+I)%I * J + (j-1+J)%J]));
	omp_set_lock(&(locks[(i-2+I)%I * J + j]));
	omp_set_lock(&(locks[(i-2+I)%I * J + (j+1)%J]));
	omp_set_lock(&(locks[(i-2+I)%I * J + (j+2)%J]));

	omp_set_lock(&(locks[(i-1+I)%I * J + (j-2+J)%J]));
	omp_set_lock(&(locks[(i-1+I)%I * J + (j-1+J)%J]));
	omp_set_lock(&(locks[(i-1+I)%I * J + j]));
	omp_set_lock(&(locks[(i-1+I)%I * J + (j+1)%J]));
	omp_set_lock(&(locks[(i-1+I)%I * J + (j+2)%J]));

	omp_set_lock(&(locks[i * J + (j-2+J)%J]));
	omp_set_lock(&(locks[i * J + (j-1+J)%J]));
	omp_set_lock(&(locks[i * J + j]));
	omp_set_lock(&(locks[i * J + (j+1)%J]));
	omp_set_lock(&(locks[i * J + (j+2)%J]));

	omp_set_lock(&(locks[(i+1)%I * J + (j-2+J)%J]));
	omp_set_lock(&(locks[(i+1)%I * J + (j-1+J)%J]));
	omp_set_lock(&(locks[(i+1)%I * J + j]));
	omp_set_lock(&(locks[(i+1)%I * J + (j+1)%J]));
	omp_set_lock(&(locks[(i+1)%I * J + (j+2)%J]));

	omp_set_lock(&(locks[(i+2)%I * J + (j-2+J)%J]));
	omp_set_lock(&(locks[(i+2)%I * J + (j-1+J)%J]));
	omp_set_lock(&(locks[(i+2)%I * J + j]));
	omp_set_lock(&(locks[(i+2)%I * J + (j+1)%J]));
	omp_set_lock(&(locks[(i+2)%I * J + (j+2)%J]));

	return;
}

void square24_unlock_neigh(CAT_Index index, omp_lock_t *locks) {
	int I = header->arraySizeI;
	int J = header->arraySizeJ;
	
	int i = index.i;
	int j = index.j;

	omp_unset_lock(&(locks[(i-2+I)%I * J + (j-2+J)%J]));
	omp_unset_lock(&(locks[(i-2+I)%I * J + (j-1+J)%J]));
	omp_unset_lock(&(locks[(i-2+I)%I * J + j]));
	omp_unset_lock(&(locks[(i-2+I)%I * J + (j+1)%J]));
	omp_unset_lock(&(locks[(i-2+I)%I * J + (j+2)%J]));

	omp_unset_lock(&(locks[(i-1+I)%I * J + (j-2+J)%J]));
	omp_unset_lock(&(locks[(i-1+I)%I * J + (j-1+J)%J]));
	omp_unset_lock(&(locks[(i-1+I)%I * J + j]));
	omp_unset_lock(&(locks[(i-1+I)%I * J + (j+1)%J]));
	omp_unset_lock(&(locks[(i-1+I)%I * J + (j+2)%J]));

	omp_unset_lock(&(locks[i * J + (j-2+J)%J]));
	omp_unset_lock(&(locks[i * J + (j-1+J)%J]));
	omp_unset_lock(&(locks[i * J + j]));
	omp_unset_lock(&(locks[i * J + (j+1)%J]));
	omp_unset_lock(&(locks[i * J + (j+2)%J]));

	omp_unset_lock(&(locks[(i+1)%I * J + (j-2+J)%J]));
	omp_unset_lock(&(locks[(i+1)%I * J + (j-1+J)%J]));
	omp_unset_lock(&(locks[(i+1)%I * J + j]));
	omp_unset_lock(&(locks[(i+1)%I * J + (j+1)%J]));
	omp_unset_lock(&(locks[(i+1)%I * J + (j+2)%J]));

	omp_unset_lock(&(locks[(i+2)%I * J + (j-2+J)%J]));
	omp_unset_lock(&(locks[(i+2)%I * J + (j-1+J)%J]));
	omp_unset_lock(&(locks[(i+2)%I * J + j]));
	omp_unset_lock(&(locks[(i+2)%I * J + (j+1)%J]));
	omp_unset_lock(&(locks[(i+2)%I * J + (j+2)%J]));

	return;
}
#endif

struct topology square24 = {
	.name = "SQUARE24",
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
	.GetCoord = square24_get_coord,
	.GetMetricSize = square24_get_metric_size,
	.GetIndex = square24_get_index,
	.GetArraySize = square24_get_array_size,
	.SquareDistance = square24_square_distance,
#ifdef USE_OMP
	.LockNeighbors = square24_lock_neigh,
	.UnlockNeighbors = square24_unlock_neigh,
#endif
	.GetNeighbors = square24_get_neigh,
	.PutNeighbors = square24_put_neigh
};

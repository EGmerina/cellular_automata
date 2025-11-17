#include "../topology.h"

const double sqrt3 = 1.732050807568877;
const double sqrt3half = 0.866025403784439;
const double sqrt3inverse = 0.577350269189626;
const double oneHalf = 0.5;

void hexagon_get_neighbors(CAT_Index index, void *n) {
	int sizeI = header->arraySizeI;
	int sizeJ = header->arraySizeJ;
	int cellSize = header->cellSize;
	char *n1;
	n1 = n;

	int i = index.i;
	int j = index.j;

	CAT_GetCell2D(n1 + 0 * cellSize, i, j);
	CAT_GetCell2D(n1 + 1 * cellSize, (i-1+sizeI)%sizeI, (j+(i&1))%sizeJ);
	CAT_GetCell2D(n1 + 2 * cellSize, i, (j+1)%sizeJ);
	CAT_GetCell2D(n1 + 3 * cellSize, (i+1)%sizeI, (j+(i&1))%sizeJ);
	CAT_GetCell2D(n1 + 4 * cellSize, (i+1)%sizeI, (j-1+(i&1)+sizeJ)%sizeJ);
	CAT_GetCell2D(n1 + 5 * cellSize, i, (j-1+sizeJ)%sizeJ);
	CAT_GetCell2D(n1 + 6 * cellSize, (i-1+sizeI)%sizeI, (j-1+(i&1)+sizeJ)%sizeJ);

	return;
}

void hexagon_put_neighbors(CAT_Index index, void *n) {
	int sizeI = header->arraySizeI;
	int sizeJ = header->arraySizeJ;
	int cellSize = header->cellSize;
	char *n1;
	n1 = n;

	int i = index.i;
	int j = index.j;

	CAT_PutCell2D(n1 + 0 * cellSize, i, j);
	CAT_PutCell2D(n1 + 1 * cellSize, (i-1+sizeI)%sizeI, (j+(i&1))%sizeJ);
	CAT_PutCell2D(n1 + 2 * cellSize, i, (j+1)%sizeJ);
	CAT_PutCell2D(n1 + 3 * cellSize, (i+1)%sizeI, (j+(i&1))%sizeJ);
	CAT_PutCell2D(n1 + 4 * cellSize, (i+1)%sizeI, (j-1+(i&1)+sizeJ)%sizeJ);
	CAT_PutCell2D(n1 + 5 * cellSize, i, (j-1+sizeJ)%sizeJ);
	CAT_PutCell2D(n1 + 6 * cellSize, (i-1+sizeI)%sizeI, (j-1+(i&1)+sizeJ)%sizeJ);

	return;
}

CAT_Coord hexagon_get_coord(CAT_Index index) {
	CAT_Coord coord;
	coord.x = (double) index.i * sqrt3half / header->cellsPerMeter;
	coord.y = ((double) index.j + oneHalf * (double)(index.i&1)) / header->cellsPerMeter;
	return coord;
}

CAT_Coord hexagon_get_metric_size(void) {
	CAT_Coord coord;
	coord.x = (double) (header->arraySizeI - 1) * sqrt3half / header->cellsPerMeter;
	coord.y = (double) (header->arraySizeJ - 1) / header->cellsPerMeter;
	return coord;
}

CAT_Index hexagon_get_index(CAT_Coord coord) {
	CAT_Index index;
	double x = coord.x * header->cellsPerMeter;
	double y = coord.y * header->cellsPerMeter;
	int i0 = 2 * (int)(x * sqrt3inverse), j0 = (int) y;
	double dx = x - i0 * sqrt3 * oneHalf, dy = y - j0;
	if (dy < (3 - sqrt3 * dx) &&
		dy < (sqrt3 * dx) &&
		dy >= (1 - sqrt3 * dx) &&
		dy >= (sqrt3 * dx - 2)) {
		index.i = (i0 + 1);
		index.j = j0;
		return index;
	}
	if (dx >= oneHalf * sqrt3)
		i0 += 2;
	if (dy >= oneHalf)
		j0++;
	index.i = i0;
	index.j = j0;
	return index;
}

CAT_Index hexagon_get_array_size(void) {
	CAT_Index index;
	index.i = header->arraySizeI;
	index.j = header->arraySizeJ;
	return index;
}

double hexagon_square_distance(CAT_Index index1, CAT_Index index2) {
	CAT_Coord coord1, coord2;
	coord1 = hexagon_get_coord(index1);
	coord2 = hexagon_get_coord(index2);
	double dx = coord2.x - coord1.x;
	double dy = coord2.y - coord1.y;
	return dx * dx + dy * dy;
}

#ifdef USE_OMP
void hexagon_lock_neighbors(CAT_Index index, omp_lock_t *locks) {
	int I = header->arraySizeI;
	int J = header->arraySizeJ;
	
	int i = index.i;
	int j = index.j;

	omp_set_lock(&(locks[i * J + j]));
	omp_set_lock(&(locks[(i-1+I)%I * J + ((j+(i&1))%J)]));
	omp_set_lock(&(locks[i * J + ((j+1)%J)]));
	omp_set_lock(&(locks[(i+1)%I * J + ((j+(i&1))%J)]));
	omp_set_lock(&(locks[(i+1)%I * J + ((j-1+(i&1)+J)%J)]));
	omp_set_lock(&(locks[i * J + ((j-1+J)%J)]));
	omp_set_lock(&(locks[(i-1+I)%I * J + ((j-1+(i&1)+J)%J)]));

	return;
}

void hexagon_unlock_neighbors(CAT_Index index, omp_lock_t *locks) {
	int I = header->arraySizeI;
	int J = header->arraySizeJ;
	
	int i = index.i;
	int j = index.j;

	omp_unset_lock(&(locks[i * J + j]));
	omp_unset_lock(&(locks[(i-1+I)%I * J + ((j+(i&1))%J)]));
	omp_unset_lock(&(locks[i * J + ((j+1)%J)]));
	omp_unset_lock(&(locks[(i+1)%I * J + ((j+(i&1))%J)]));
	omp_unset_lock(&(locks[(i+1)%I * J + ((j-1+(i&1)+J)%J)]));
	omp_unset_lock(&(locks[i * J + ((j-1+J)%J)]));
	omp_unset_lock(&(locks[(i-1+I)%I * J + ((j-1+(i&1)+J)%J)]));

	return;
}
#endif

struct topology hexagon = {
	.name = "HEXAGON",
	.dimension = CAT_TOP_DIMENSION,
	.neighborsNumber = CAT_TOP_NEIGHBORS_NUMBER,
	.latticePeriod.i = 2,
	.latticePeriod.j = 1,
	.latticePeriod.k = 1,
	.latticePeriod.l = 1,
	.borderWidth.i = 1,
	.borderWidth.j = 1,
	.borderWidth.k = 1,
	.borderWidth.l = 1,
	.GetCoord = hexagon_get_coord,
	.GetMetricSize = hexagon_get_metric_size,
	.GetIndex = hexagon_get_index,
	.GetArraySize = hexagon_get_array_size,
	.SquareDistance = hexagon_square_distance,
#ifdef USE_OMP
	.LockNeighbors = hexagon_lock_neighbors,
	.UnlockNeighbors = hexagon_unlock_neighbors,
#endif
	.GetNeighbors = hexagon_get_neighbors,
	.PutNeighbors = hexagon_put_neighbors
};


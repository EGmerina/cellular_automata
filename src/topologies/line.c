#include "../topology.h"

void line_get_neighbors(CAT_Index index, void *n) {
	int sizeI = header->arraySizeI;
	int cellSize = header->cellSize;
	char *n1;
	n1 = n;

	int i = index.i;

//	CAT_GetCell1D(n1, i);
//	CAT_GetCell1D(n1 + cellSize, (i-1+sizeI)%sizeI);
//	CAT_GetCell1D(n1 + 2 * cellSize, (i-1+sizeI)%sizeI);

	return;
}

void line_put_neighbors(CAT_Index index, void *n) {
	// TODO: implement this
	return;
}

struct topology line = {
	.name = "LINE",
	.dimension = CAT_TOP_DIMENSION,
	.neighborsNumber = CAT_TOP_NEIGHBORS_NUMBER,
	.latticePeriod.i = 1,
	.latticePeriod.j = 1,
	.latticePeriod.k = 1,
	.latticePeriod.l = 1,
	.borderWidth.i = 1,
	.borderWidth.j = 1,
	.borderWidth.k = 1,
	.borderWidth.l = 1,
	.GetCoord = NULL,
	.GetIndex = NULL,
	.GetNeighbors = line_get_neighbors,
	.PutNeighbors = NULL
};

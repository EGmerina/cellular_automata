#include "../topology.h"

const double sqrt2 = 1.414213562373095;
const double sqrt3inverse = 0.577350269189626;
const double oneHalf = 0.499999999999999;
const double h = 0.707106781186548; //sqrt2half

#define _3D

void cube12_get_neigh(CAT_Index index, void* n) {

    int sizeI = header->arraySizeI;
    int sizeJ = header->arraySizeJ;
    int sizeK = header->arraySizeK;
    int cellSize = header->cellSize;
    char* n1;
    n1 = n;

    int i = index.i;
    int j = index.j;
    int k = index.k;
    
    int i_plus = (i+1)%sizeI;
    int i_minus = (i-1+sizeI)%sizeI;
    int j_plus = (j+1)%sizeJ;
    int j_minus = (j-1+sizeJ)%sizeJ;
    int k_plus = (k+((j&1)^(i&1)))%sizeK;
    int k_minus = (k+(-1+((j&1)^(i&1)))+sizeK)%sizeK;

    CAT_GetCell3D(n1, i, j, k);

    CAT_GetCell3D(n1 + 1 * cellSize, i_plus, j_plus, k);
    CAT_GetCell3D(n1 + 2 * cellSize, i_plus, j_minus, k);
    CAT_GetCell3D(n1 + 3 * cellSize, i, j_plus, k_plus);
    CAT_GetCell3D(n1 + 4 * cellSize, i, j_plus, k_minus);

    CAT_GetCell3D(n1 + 5 * cellSize, i_plus, j, k_plus);
    CAT_GetCell3D(n1 + 6 * cellSize, i_minus, j, k_plus);
    CAT_GetCell3D(n1 + 7 * cellSize, i_minus, j_minus, k);
    CAT_GetCell3D(n1 + 8 * cellSize, i_minus, j_plus, k);

    CAT_GetCell3D(n1 + 9 * cellSize, i, j_minus, k_minus);
    CAT_GetCell3D(n1 + 10 * cellSize, i, j_minus, k_plus);
    CAT_GetCell3D(n1 + 11 * cellSize, i_minus, j, k_minus);
    CAT_GetCell3D(n1 + 12 * cellSize, i_plus, j, k_minus);
    return;
}

void cube12_put_neigh(CAT_Index index, void* n) {
    int sizeI = header->arraySizeI;
    int sizeJ = header->arraySizeJ;
    int sizeK = header->arraySizeK;
    int cellSize = header->cellSize;
    char* n1;
    n1 = n;

    int i = index.i;
    int j = index.j;
    int k = index.k;

    int i_plus = (i+1)%sizeI;
    int i_minus = (i-1+sizeI)%sizeI;
    int j_plus = (j+1)%sizeJ;
    int j_minus = (j-1+sizeJ)%sizeJ;
    int k_plus = (k+((j&1)^(i&1)))%sizeK;
    int k_minus = (k+(-1+((j&1)^(i&1)))+sizeK)%sizeK;

    CAT_PutCell3D(n1, i, j, k);

    CAT_PutCell3D(n1 + 1 * cellSize, i_plus, j_plus, k);
    CAT_PutCell3D(n1 + 2 * cellSize, i_plus, j_minus, k);
    CAT_PutCell3D(n1 + 3 * cellSize, i, j_plus, k_plus);
    CAT_PutCell3D(n1 + 4 * cellSize, i, j_plus, k_minus);

    CAT_PutCell3D(n1 + 5 * cellSize, i_plus, j, k_plus);
    CAT_PutCell3D(n1 + 6 * cellSize, i_minus, j, k_plus);
    CAT_PutCell3D(n1 + 7 * cellSize, i_minus, j_minus, k);
    CAT_PutCell3D(n1 + 8 * cellSize, i_minus, j_plus, k);

    CAT_PutCell3D(n1 + 9 * cellSize, i, j_minus, k_minus);
    CAT_PutCell3D(n1 + 10 * cellSize, i, j_minus, k_plus);
    CAT_PutCell3D(n1 + 11 * cellSize, i_minus, j, k_minus);
    CAT_PutCell3D(n1 + 12 * cellSize, i_plus, j, k_minus);
    return;
}

void cube12_get_neigh_sav(CAT_Index index, void* n) {

    int sizeI = header->arraySizeI;
    int sizeJ = header->arraySizeJ;
    int sizeK = header->arraySizeK;
    int cellSize = header->cellSize;
    char* n1;
    n1 = n;

    int i = index.i;
    int j = index.j;
    int k = index.k;

    CAT_GetCell3D(n1 + 0 * cellSize, i, j, k);

    CAT_GetCell3D(n1 + 1 * cellSize, (i+1)%sizeI, (j+1)%sizeJ, k);
    CAT_GetCell3D(n1 + 2 * cellSize,  (i+1)%sizeI, (j-1+sizeJ)%sizeJ, k);
    CAT_GetCell3D(n1 + 3 * cellSize, i, (j+1)%sizeJ, (k+((j&1)^(i&1)))%sizeK);
    CAT_GetCell3D(n1 + 4 * cellSize, i, (j+1)%sizeJ, (k+(-1+((j&1)^(i&1)))+sizeK)%sizeK);

    CAT_GetCell3D(n1 + 5 * cellSize, (i+1)%sizeI, j, (k+((j&1)^(i&1)))%sizeK);
    CAT_GetCell3D(n1 + 6 * cellSize, (i-1+sizeI)%sizeI, j, (k+((j&1)^(i&1)))%sizeK);
    CAT_GetCell3D(n1 + 7 * cellSize, (i-1+sizeI)%sizeI, (j-1+sizeJ)%sizeJ, k);
    CAT_GetCell3D(n1 + 8 * cellSize, (i-1+sizeI)%sizeI, (j+1)%sizeJ, k);

    CAT_GetCell3D(n1 + 9 * cellSize, i, (j-1+sizeJ)%sizeJ, (k+(-1+((j&1)^(i&1)))+sizeK)%sizeK);
    CAT_GetCell3D(n1 + 10 * cellSize, i, (j-1+sizeJ)%sizeJ, (k+((j&1)^(i&1)))%sizeK);
    CAT_GetCell3D(n1 + 11 * cellSize, (i-1+sizeI)%sizeI, j, (k+(-1+((j&1)^(i&1)))+sizeK)%sizeK);
    CAT_GetCell3D(n1 + 12 * cellSize, (i+1)%sizeI, j, (k+(-1+((j&1)^(i&1)))+sizeK)%sizeK);
    return;
}

void cube12_put_neigh_sav(CAT_Index index, void* n) {
    int sizeI = header->arraySizeI;
    int sizeJ = header->arraySizeJ;
    int sizeK = header->arraySizeK;
    int cellSize = header->cellSize;
    char* n1;
    n1 = n;

    int i = index.i;
    int j = index.j;
    int k = index.k;

    CAT_PutCell3D(n1, i, j, k);

    CAT_PutCell3D(n1 + 1 * cellSize, (i+1)%sizeI, (j+1)%sizeJ, k);
    CAT_PutCell3D(n1 + 2 * cellSize,  (i+1)%sizeI, (j-1+sizeJ)%sizeJ, k);
    CAT_PutCell3D(n1 + 3 * cellSize, i, (j+1)%sizeJ, (k+((j&1)^(i&1)))%sizeK);
    CAT_PutCell3D(n1 + 4 * cellSize, i, (j+1)%sizeJ, (k+(-1+((j&1)^(i&1)))+sizeK)%sizeK);

    CAT_PutCell3D(n1 + 5 * cellSize, (i+1)%sizeI, j, (k+((j&1)^(i&1)))%sizeK);
    CAT_PutCell3D(n1 + 6 * cellSize, (i-1+sizeI)%sizeI, j, (k+((j&1)^(i&1)))%sizeK);
    CAT_PutCell3D(n1 + 7 * cellSize, (i-1+sizeI)%sizeI, (j-1+sizeJ)%sizeJ, k);
    CAT_PutCell3D(n1 + 8 * cellSize, (i-1+sizeI)%sizeI, (j+1)%sizeJ, k);

    CAT_PutCell3D(n1 + 9 * cellSize, i, (j-1+sizeJ)%sizeJ, (k+(-1+((j&1)^(i&1)))+sizeK)%sizeK);
    CAT_PutCell3D(n1 + 10 * cellSize, i, (j-1+sizeJ)%sizeJ, (k+((j&1)^(i&1)))%sizeK);
    CAT_PutCell3D(n1 + 11 * cellSize, (i-1+sizeI)%sizeI, j, (k+(-1+((j&1)^(i&1)))+sizeK)%sizeK);
    CAT_PutCell3D(n1 + 12 * cellSize, (i+1)%sizeI, j, (k+(-1+((j&1)^(i&1)))+sizeK)%sizeK);
    return;
}

CAT_Coord cube12_get_coord (CAT_Index index) {
    CAT_Coord coord;
    coord.x = (double) index.i * h / header->cellsPerMeter;
    coord.y = ((double) index.j * h) / header->cellsPerMeter;
    coord.z = ((double) (2 * index.k + ((index.i&1)^(index.j&1))) * h) / header->cellsPerMeter;
    return coord;
}

double cube12_square_distance(CAT_Index index1, CAT_Index index2) {
    CAT_Coord coord1, coord2;
    coord1 = cube12_get_coord(index1);
    coord2 = cube12_get_coord(index2);
    double dx = coord2.x - coord1.x;
    double dy = coord2.y - coord1.y;
    double dz = coord2.z - coord1.z;
    return dx * dx + dy * dy + dz * dz;
}

CAT_Index cube12_get_array_size(void) {
    CAT_Index index;
    index.i = header->arraySizeI;
    index.j = header->arraySizeJ;
    index.k = header->arraySizeK;
    return index;
}

CAT_Coord cube12_get_metric_size(void) {
    CAT_Index index;
    index.i = header->arraySizeI - 1;
    index.j = header->arraySizeJ - 1;
    index.k = header->arraySizeK - 1;
    return cube12_get_coord (index);
}

CAT_Index cube12_get_index(CAT_Coord coord) {
    CAT_Index index;
    double x = (coord.x * 1 / h + oneHalf) * header->cellsPerMeter;
    double y = (coord.y * 1 / h + oneHalf) * header->cellsPerMeter;
    double z = (coord.z * 1 / (2 * h) + oneHalf) * header->cellsPerMeter;
    index.i = (int) x;
    index.j = (int) y;
    index.k = (int) z;
    return index;
}

struct topology cube12 = {
        .name = "CUBE12",
        .dimension = CAT_TOP_DIMENSION,
        .neighborsNumber = CAT_TOP_NEIGHBORS_NUMBER,
        .latticePeriod.i = 2,
        .latticePeriod.j = 2,
        .latticePeriod.k = 2,
        .latticePeriod.l = 1,
        .borderWidth.i = 1,
        .borderWidth.j = 1,
        .borderWidth.k = 1,
        .borderWidth.l = 1,
        .GetCoord = cube12_get_coord,
        .GetMetricSize = cube12_get_metric_size,
        .GetIndex = cube12_get_index,
        .GetArraySize = cube12_get_array_size,
        .SquareDistance = cube12_square_distance,
        .GetNeighbors = cube12_get_neigh,
        .PutNeighbors = cube12_put_neigh
};

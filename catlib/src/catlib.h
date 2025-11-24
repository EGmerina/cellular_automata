#pragma once

extern int CAT_Rand(int RandMax);
extern void CAT_Srand(unsigned int);

typedef struct CAT_Index {
	int i;
	int j;
	int k;
	int l;
} CAT_Index;

typedef struct CAT_Coord {
	double x;
	double y;
	double z;
	double w;
} CAT_Coord;

void CAT_PrintEnvironment(void);
CAT_Coord CAT_InitPreprocessor(int cellSize, int globalSize, double cellsPerMeter, CAT_Coord coord);
int CAT_InitSimulator(char *filename);
int CAT_InitPostprocessor(char *filename);

void CAT_GetCell2D(void *cellValue, int i, int j);
void CAT_GetCell3D(void *cellValue, int i, int j, int k);
void CAT_GetCell(void *cellValue, int i, int j); // temporary 2D only
void CAT_PutCell(void *cellValue, int i, int j); // temporary 2D only
void CAT_PutCell2D(void *cellValue, int i, int j);
void CAT_PutCell3D(void *cellValue, int i, int j, int k);

int _CAT_GetNeighborsNumber(void);
CAT_Coord CAT_GetCoord(CAT_Index);
CAT_Coord CAT_GetMetricSize(void);
CAT_Index CAT_GetIndex(CAT_Coord);
CAT_Index CAT_GetArraySize(void);
double CAT_SquareDistance(CAT_Index, CAT_Index);
double CAT_GetCellsPerMeter(void);
int CAT_GetIterationsDone(void);

int CAT_Iterate(int (*cellTransition)(void*));

int CAT_GetNumThreads(void);
int CAT_SetNumThreads(int numThreads);

int CAT_FinalizePreprocessor(char *filename);
int CAT_FinalizeSimulator(char *filename);
int CAT_FinalizePostprocessor();


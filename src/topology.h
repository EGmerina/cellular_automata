#pragma once

#include "catlib.h"
#include "catparallel.h"

#define LINE     10
#define SQUARE4  20
#define SQUARE8  21
#define HEXAGON  22
#define SQUARE24 26
#define CUBE6    30
#define CUBE12   32
#define CUBE26   36

#if TOPOLOGY == LINE
	#define CAT_Topology line
	#define CAT_TOP_DIMENSION 1
	#define CAT_TOP_NEIGHBORS_NUMBER 2
	#include "topologies/line.h"
#endif

#if TOPOLOGY == SQUARE4
	#define CAT_Topology square4
	#define CAT_TOP_DIMENSION 2
	#define CAT_TOP_NEIGHBORS_NUMBER 4
	#include "topologies/square4.h"
#endif

#if TOPOLOGY == SQUARE8
	#define CAT_Topology square8
	#define CAT_TOP_DIMENSION 2
	#define CAT_TOP_NEIGHBORS_NUMBER 8
	#include "topologies/square8.h"
#endif

#if TOPOLOGY == HEXAGON
	#define CAT_Topology hexagon
	#define CAT_TOP_DIMENSION 2
	#define CAT_TOP_NEIGHBORS_NUMBER 6
	#include "topologies/hexagon.h"
#endif

#if TOPOLOGY == SQUARE24
	#define CAT_Topology square24
	#define CAT_TOP_DIMENSION 2
	#define CAT_TOP_NEIGHBORS_NUMBER 24
	#include "topologies/square24.h"
#endif

#if TOPOLOGY == CUBE6
	#define CAT_Topology cube6
	#define CAT_TOP_DIMENSION 3
	#define CAT_TOP_NEIGHBORS_NUMBER 6
//	#include "topologies/cube6.h"
#endif

#if TOPOLOGY == CUBE12
	#define CAT_Topology cube12
	#define CAT_TOP_DIMENSION 3
	#define CAT_TOP_NEIGHBORS_NUMBER 12
	#include "topologies/cube12.h"
#endif

#if TOPOLOGY == CUBE26
	#define CAT_Topology cube26
	#define CAT_TOP_DIMENSION 3
	#define CAT_TOP_NEIGHBORS_NUMBER 26
//	#include "topologies/cube26.h"
#endif

struct topology {
	char name[1000];
	int neighborsNumber;
	int dimension;
	CAT_Index latticePeriod;
	CAT_Index borderWidth;

	CAT_Coord (*GetCoord)(CAT_Index);
	CAT_Index (*GetIndex)(CAT_Coord);
	
	CAT_Coord (*GetMetricSize)(void);
	CAT_Index (*GetArraySize)(void);

	double (*SquareDistance)(CAT_Index, CAT_Index);

	void (*GetNeighbors)(CAT_Index, void *);
	void (*PutNeighbors)(CAT_Index, void *);

#ifdef USE_OMP
	void (*LockNeighbors)(CAT_Index, omp_lock_t *locks);
	void (*UnlockNeighbors)(CAT_Index, omp_lock_t *locks);
#endif
};


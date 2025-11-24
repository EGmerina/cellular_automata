#include <iostream>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <cmath>
using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#include "fhpmp.hpp"

#define PROGRESS_BAR 100

extern int CAT_Rand(int);

inline int abs1(int x) { return (x<0?-x:x); }

inline int countSubVariants(int N, int n1, int n2, int n3)
{
	int K = N - abs1(n1) - abs1(n2) - abs1(n3);
	
	if (K<0 || K%2==1) return 0;
	K /= 2;
	
	//C^2_K/2
	return ((K+2)*(K+1))/2;
}

void randomPartition(int K,int& K1,int& K2,int& K3)
{
	int cnt = ((K+2)*(K+1))/2;
	int rnd = CAT_Rand(cnt)+1;
	
	double x = (sqrt(1.0l + 8.0l*rnd)-1.0l)/2.0l;
	//step1 >= step2, step1,step2>=0
	int step1 = ceil(x);
	int step2 = rnd - ((step1-1)*step1)/2;
	step1--;
	step2--;
	//
	K1 = step2;
	K2 = step1-step2;
	K3 = K-K1-K2;
}

void randomPartition2(int K,int& K1,int& K2,int& K3)
{
	int cnt = ((K+2)*(K+1))/2;
	
	int diag = CAT_Rand(cnt)+1;
	if (diag<=K+1)
	{
		int r = CAT_Rand(K+1);
		K1=r;
		K2=0;
		K3=K-r;
	}
	else
	{
		int r1,r2;
		while(1)
		{
			r1 = CAT_Rand(K+1);
			r2 = CAT_Rand(K+1);
			if (r1!=r2) break;
		}
		if (r2<r1)
		{
			int t = r2;
			r2 = r1;
			r1 = t;
		}
		
		K1 = r1;
		K2 = r2-r1;
		K3 = K-K1-K2;
	}
}

void chooseSubVariant(int N, int n1, int n2, int n3, vector<int>& ret)
{
	int K = N - abs1(n1) - abs1(n2) - abs1(n3);
	K /= 2;
	
	int K1,K2,K3;
	randomPartition(K,K1,K2,K3);
	
	ret[0] = (n2>0 ? n2 : 0) + K1;
	ret[3] = (n2<0 ?-n2 : 0) + K1;
	
	ret[1] = (n3>0 ? n3 : 0) + K2;
	ret[4] = (n3<0 ?-n3 : 0) + K2;
	
	ret[5] = (n1>0 ? n1 : 0) + K3;
	ret[2] = (n1<0 ?-n1 : 0) + K3;
}

void comb(vector<int>& dir, vector<int>& ret)
{
	ret.resize(6);
	
	int n1 = dir[5]-dir[2];
	int n2 = dir[0]-dir[3];
	int n3 = dir[1]-dir[4];
	
	int N = 0;
	for(int i=0; i<6; ++i)
		N += dir[i];
	
	n1 += n2;
	n3 += n2;
	
	vector<int> sub, way;
	int subSum = 0;
	
	//count subvariants
	for(int W=-N; W<=N; ++W)
	{
		int N1 = n1-W;
		int N2 = W;
		int N3 = n3-W;
		
		int cnt = countSubVariants(N,N1,N2,N3);
		if (cnt!=0)
		{
			sub.push_back(cnt);
			way.push_back(W);
			subSum += cnt;
		}
	}
	
	//gen random number in [1..subSum]
	int rnd = CAT_Rand(subSum)+1;
	int sz = sub.size();
	int ch = -1;
	//choose one of the variant
	for(int i=0; i<sz; ++i)
	{
		if (rnd <= sub[i])
		{
			ch = i;
			break;
		}
		rnd -= sub[i];
	}
	
	assert(ch!=-1);
	
	int w = way[ch];
	int N1 = n1-w;
	int N2 = w;
	int N3 = n3-w;
	
	chooseSubVariant(N,N1,N2,N3,ret);
}

void set_cell(int i, int j)
{
    hexCell cell;
    cell.countCellsDirection0 = 0;
    cell.countCellsDirection1 = 0;
    cell.countCellsDirection2 = 0;
    cell.countCellsDirection3 = 0;
    cell.countCellsDirection4 = 0;
    cell.countCellsDirection5 = 0;
    cell.cellType = 1;
	CAT_PutCell(&cell, i, j);
}

int print_cell(void *n)
{
    hexCell *cell = (hexCell*) n;
    printf("%5d, %5d, %5d, %5d, %5d, %5d --- type %2d\n",
    cell->countCellsDirection0,
    cell->countCellsDirection1,
    cell->countCellsDirection2,
    cell->countCellsDirection3,
    cell->countCellsDirection4,
    cell->countCellsDirection5,
    cell->cellType);fflush(stdout);
    return 0;
}

int print_nonempty_cell(void *n)
{
    hexCell *cell = (hexCell*) n;
    CAT_Index arraySize = CAT_GetArraySize();
    static int cellNumber = 0;
    int iterNumber = cellNumber / (arraySize.i * arraySize.j);
    int i = (cellNumber / arraySize.j) % arraySize.i;
    int j = cellNumber % arraySize.j;
    if(cell->countCellsDirection0
    || cell->countCellsDirection1
    || cell->countCellsDirection2
    || cell->countCellsDirection3
    || cell->countCellsDirection4
    || cell->countCellsDirection5
	|| cell->cellType)
    {
		printf("%3d: (%3d, %3d) --- %3d, %3d, %3d, %3d, %3d, %3d --- type %2d\n",
		iterNumber, i, j,
		cell->countCellsDirection0,
		cell->countCellsDirection1,
		cell->countCellsDirection2,
		cell->countCellsDirection3,
		cell->countCellsDirection4,
		cell->countCellsDirection5,
		cell->cellType);fflush(stdout);
	}
    cellNumber++;
    return 0;
}

int print_nonempty_neighbors(void *n)
{
    hexCell *cell = (hexCell*) n;
    CAT_Index arraySize = CAT_GetArraySize();
    static int cellNumber = 0;
    int iterNumber = cellNumber / (arraySize.i * arraySize.j);
    int i = (cellNumber / arraySize.j) % arraySize.i;
    int j = cellNumber % arraySize.j;
    for (int k = 0; k < 8; k++)
    {
		if((cell + k)->countCellsDirection0
		|| (cell + k)->countCellsDirection1
		|| (cell + k)->countCellsDirection2
		|| (cell + k)->countCellsDirection3
		|| (cell + k)->countCellsDirection4
		|| (cell + k)->countCellsDirection5
		|| (cell + k)->cellType)
		{
			printf("%3d: (%3d, %3d, %3d) --- %3d, %3d, %3d, %3d, %3d, %3d --- type %2d\n",
			iterNumber, i, j, k,
			(cell + k)->countCellsDirection0,
			(cell + k)->countCellsDirection1,
			(cell + k)->countCellsDirection2,
			(cell + k)->countCellsDirection3,
			(cell + k)->countCellsDirection4,
			(cell + k)->countCellsDirection5,
			(cell + k)->cellType);fflush(stdout);
		}
	}
    cellNumber++;
    return 0;
}

int propagation(void *n)
{
    hexCell *cell = (hexCell*) n;
    cell->countCellsDirection0 = (cell + 2)->countCellsDirection0;
    cell->countCellsDirection1 = (cell + 1)->countCellsDirection1;
    cell->countCellsDirection2 = (cell + 6)->countCellsDirection2;
    cell->countCellsDirection3 = (cell + 5)->countCellsDirection3;
    cell->countCellsDirection4 = (cell + 4)->countCellsDirection4;
    cell->countCellsDirection5 = (cell + 3)->countCellsDirection5;
    return 0;
}

hexCell collision_conventional(hexCell cell)
{
	vector<int> dir(6), ret(6);
	dir[0] = cell.countCellsDirection0;
	dir[1] = cell.countCellsDirection1;
	dir[2] = cell.countCellsDirection2;
	dir[3] = cell.countCellsDirection3;
	dir[4] = cell.countCellsDirection4;
	dir[5] = cell.countCellsDirection5;
	int s1 = dir[0] + dir[1] + dir[2] + dir[3] + dir[4] + dir[5];
	comb(dir, ret);
	int s2 = ret[0] + ret[1] + ret[2] + ret[3] + ret[4] + ret[5];
	if(s1 != s2)
	{
		fprintf(stderr,"ERROR: s1=%d, s2=%d\n", s1, s2);
		return cell;
	}
	cell.countCellsDirection0 = ret[0];
	cell.countCellsDirection1 = ret[1];
	cell.countCellsDirection2 = ret[2];
	cell.countCellsDirection3 = ret[3];
	cell.countCellsDirection4 = ret[4];
	cell.countCellsDirection5 = ret[5];
	return cell;
}

hexCell collision_wall(hexCell cell)
{
	hexCell cell1;
	cell1.countCellsDirection0 = cell.countCellsDirection3;
	cell1.countCellsDirection1 = cell.countCellsDirection4;
	cell1.countCellsDirection2 = cell.countCellsDirection5;
	cell1.countCellsDirection3 = cell.countCellsDirection0;
	cell1.countCellsDirection4 = cell.countCellsDirection1;
	cell1.countCellsDirection5 = cell.countCellsDirection2;
	return cell1;
}

hexCell collision_inlet(hexCell cell)
{
	const int INLET_DIR_PRESS = 10;
	cell = collision_wall(cell);
	cell.countCellsDirection0 = cell.countCellsDirection0 < INLET_DIR_PRESS ? INLET_DIR_PRESS : cell.countCellsDirection0;
	cell.countCellsDirection1 = cell.countCellsDirection1 < INLET_DIR_PRESS ? INLET_DIR_PRESS : cell.countCellsDirection1;
	cell.countCellsDirection2 = cell.countCellsDirection2 < INLET_DIR_PRESS ? INLET_DIR_PRESS : cell.countCellsDirection2;
	cell.countCellsDirection3 = cell.countCellsDirection3 < INLET_DIR_PRESS ? INLET_DIR_PRESS : cell.countCellsDirection3;
	cell.countCellsDirection4 = cell.countCellsDirection4 < INLET_DIR_PRESS ? INLET_DIR_PRESS : cell.countCellsDirection4;
	cell.countCellsDirection5 = cell.countCellsDirection5 < INLET_DIR_PRESS ? INLET_DIR_PRESS : cell.countCellsDirection5;
	return cell;
}

hexCell collision_outlet(hexCell cell)
{
	const int OUTLET_DIR_PRESS = 3;
	cell = collision_wall(cell);
	cell.countCellsDirection0 = cell.countCellsDirection0 > OUTLET_DIR_PRESS ? OUTLET_DIR_PRESS : cell.countCellsDirection0;
	cell.countCellsDirection1 = cell.countCellsDirection1 > OUTLET_DIR_PRESS ? OUTLET_DIR_PRESS : cell.countCellsDirection1;
	cell.countCellsDirection2 = cell.countCellsDirection2 > OUTLET_DIR_PRESS ? OUTLET_DIR_PRESS : cell.countCellsDirection2;
	cell.countCellsDirection3 = cell.countCellsDirection3 > OUTLET_DIR_PRESS ? OUTLET_DIR_PRESS : cell.countCellsDirection3;
	cell.countCellsDirection4 = cell.countCellsDirection4 > OUTLET_DIR_PRESS ? OUTLET_DIR_PRESS : cell.countCellsDirection4;
	cell.countCellsDirection5 = cell.countCellsDirection5 > OUTLET_DIR_PRESS ? OUTLET_DIR_PRESS : cell.countCellsDirection5;
	return cell;
}

int collision(void *n)
{
    hexCell *cell = (hexCell*) n;
    hexCell myCell;
    myCell.countCellsDirection0 = cell->countCellsDirection0;
    myCell.countCellsDirection1 = cell->countCellsDirection1;
    myCell.countCellsDirection2 = cell->countCellsDirection2;
    myCell.countCellsDirection3 = cell->countCellsDirection3;
    myCell.countCellsDirection4 = cell->countCellsDirection4;
    myCell.countCellsDirection5 = cell->countCellsDirection5;
	switch(cell->cellType)
	{
	case CONVENTIONAL:
		myCell = collision_conventional(myCell);
		break;
	case INLET:
		myCell = collision_inlet(myCell);
		break;
	case OUTLET:
		myCell = collision_outlet(myCell);
		break;
	case WALL:
		myCell = collision_wall(myCell);
		break;
	}
    cell->countCellsDirection0 = myCell.countCellsDirection0;
    cell->countCellsDirection1 = myCell.countCellsDirection1;
    cell->countCellsDirection2 = myCell.countCellsDirection2;
    cell->countCellsDirection3 = myCell.countCellsDirection3;
    cell->countCellsDirection4 = myCell.countCellsDirection4;
    cell->countCellsDirection5 = myCell.countCellsDirection5;
    return 0;
}

int main(int argc, char *argv[])
{
	if(argc != 4)
    {
        printf("usage: %s file_in file_out num_of_iters\n", argv[0]);
        return 1;
    }
    char *p;
    errno = 0;
    long itersNumber = strtol(argv[3], &p, 10);
    if (errno != 0 || *p != '\0' || itersNumber > 1000000 || itersNumber < 0)
    {
        printf("Error: wrong number of iterations\n");
        return 1;
    }

    CAT_Srand(time(NULL));
    printf("Loading input file: %s\n", argv[1]);
    int flag = CAT_InitSimulator((char *)argv[1]);
    if (flag)
    {
        printf("Error: can't read input file: %s\n", argv[1]);
        return 1;
    }

    printf("Simulator started for %ld iterations with %d thread(s)\n", itersNumber, CAT_GetNumThreads()); fflush(stdout);
	int dotsNumber = itersNumber > PROGRESS_BAR ? PROGRESS_BAR : itersNumber;
   	printf("["); fflush(stdout);
   	for (int i = 0; i < dotsNumber; i++)
   		printf("."); fflush(stdout);
   	printf("]\r["); fflush(stdout);

	long long t1, t2;
	double tDiff;
	timespec curTime;
	clock_gettime(CLOCK_BOOTTIME, &curTime);
	t1 = curTime.tv_sec * 1000000000 + curTime.tv_nsec;

    for (int i = 0; i < itersNumber; i++)
    {
    	if(i % (itersNumber / dotsNumber) == 0)
    		printf("#"); fflush(stdout);

        flag = CAT_Iterate(collision);
        if (flag)
        {
            printf("\nError: %d, iteration: %d\n", flag, i);
            return 1;
        }

        flag = CAT_Iterate(propagation);
        if (flag)
        {
            printf("\nError: %d, iteration: %d\n", flag, i);
            return 1;
        }
    }

	clock_gettime(CLOCK_BOOTTIME, &curTime);
	t2 = curTime.tv_sec * 1000000000 + curTime.tv_nsec;
	tDiff = (double) (t2 - t1) / 1000000000.0;

    printf("\nTime = %g s\n", tDiff); fflush(stdout);

    printf("Saving output file: %s\n", argv[2]);
    flag = CAT_FinalizeSimulator((char *)argv[2]);
    if (flag)
    {
        printf("Error: can't write output file: %s\n", argv[2]);
        return 1;
    }

    printf("Simulator finished\n");
    return 0;
}


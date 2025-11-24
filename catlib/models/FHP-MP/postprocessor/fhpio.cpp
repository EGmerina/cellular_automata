// FHP input-output
// Version 2.0
//

#include <stdio.h>
//#include <iostream.h>
#include <string.h>
#include <stdlib.h>
#include "../fhpmp.hpp"

extern int X_size, Y_size, X2_size, Y2_size;
extern double metricX, metricY, cellsInMillimeter;
extern int N_iter, N_old, S_int;
extern int pixelsX, pixelsY;

extern long xy2l(int,int,int);		// Gets index number from coordinates
extern int l2x(long);				// Gets x-coordinate from index number
extern int l2y(long);				// Gets x-coordinate from index number
extern long xyn2l(int,int,int,int);	// Gets neighbor's index number from coordinates
extern bool makeAveragedColumns;

char ver[]="1.0";									// Version

char t1[]="FHP-MP Global State\r\nVersion ";
char t2[]="\r\nX-size = ";
char t3[]=" cells\r\nY-size = ";
char t4[]=" cells\r\nNumber of iterations = ";
char t5[]="\r\n\32";

int read_header(char *fn)
{
	int flag;
	hexCell cell;

    printf("\nReading header from %s\n", fn);
    flag = CAT_InitPostprocessor(fn);
    if (flag) {
        printf("Error: can't read header from %s\n", fn);
        return 1;
    }
    CAT_Index index = CAT_GetArraySize();
	CAT_Coord coord = CAT_GetMetricSize();
	X_size = index.i;
	Y_size = index.j;
	metricX = coord.x;
	metricY = coord.y;
	cellsInMillimeter = CAT_GetCellsPerMeter();
    flag = CAT_FinalizePostprocessor();
    if (flag) {
        printf("Error: can't finalize postprocessor\n");
        return 1;
    }
	printf("OK read header from %s\n", fn);
	return 0;
}

int read_ca_file(char *fn, unsigned char *ca)
{
	int i,j,flag;
	hexCell cell;

    printf("\nReading input file: %s\n", fn);
    flag = CAT_InitPostprocessor(fn);
    if (flag) {
        printf("Error: can't read input file: %s\n", fn);
        return 1;
    }
    CAT_Index index = CAT_GetArraySize();
//	CAT_Coord coord = CAT_GetMetricSize();
	if(index.i != X_size || index.j != Y_size)
	{
        printf("Error: array size in the .conf file %dx%d not equal to array size in the input file %dx%d\n", X_size, Y_size, index.i, index.j);
		return 5;
	}

// Reading CA
	for(i=0;i<X_size;i++)
		for(j=0;j<Y_size;j++)
		{
			CAT_GetCell(&cell, i, j);
			ca[xy2l(i,j,0)] = 0;
			ca[xy2l(i,j,1)] = cell.countCellsDirection0;
			ca[xy2l(i,j,2)] = cell.countCellsDirection1;
			ca[xy2l(i,j,3)] = cell.countCellsDirection2;
			ca[xy2l(i,j,4)] = cell.countCellsDirection3;
			ca[xy2l(i,j,5)] = cell.countCellsDirection4;
			ca[xy2l(i,j,6)] = cell.countCellsDirection5;
			ca[xy2l(i,j,7)] = cell.cellType;
		}

    flag = CAT_FinalizePostprocessor();
    if (flag) {
        printf("Error: can't finalize postprocessor\n");
        return 1;
    }
	printf("OK read input file: %s\n", fn);
	return 0;
}

int save_text_transpose(char *fn, double *averagedValue, int startX, int finishX, int startY, int finishY, int step)
{
	int i,j;
	FILE *f;

// Opening
	f=fopen(fn,"w");
	if(f==NULL)
		return 1201;

// Writing averaged value
	for(j=startY;j<=finishY;j+=step)
	{
		for(i=startX;i<=finishX;i+=step)
			if(averagedValue[i*pixelsY+j]==-100)
			{
				if(!fprintf(f,"w\t"))
					return 1203;
			}
			else
				if(!fprintf(f,"%g\t",averagedValue[i*pixelsY+j]))
					return 1203;
		if(!fprintf(f,"\n"))
			return 1204;
	}
	if(makeAveragedColumns)
	{
		if(!fprintf(f,"\n"))
			return 1204;
		for(i=startX;i<=finishX;i+=step)
		{
			double sum=0;
			int count=0;
			for(j=startY;j<=finishY;j+=step)
				if(averagedValue[i*pixelsY+j]==-100)
				{
					count=0;
					break;
				}
				else
				{
					sum+=averagedValue[i*pixelsY+j];
					count++;
				}
			if(!count)
			{
				if(!fprintf(f,"w\t"))
					return 1203;
			}
			else
				if(!fprintf(f,"%g\t",sum/count))
					return 1203;
		}
		if(!fprintf(f,"\n"))
			return 1204;
	}
	if(fclose(f))
		return 1202;
	return 0;
}

int save_text(char *fn, double *averagedValue, int startX, int finishX, int startY, int finishY, int step)
{
	int i,j;
	FILE *f;

// Opening
	f=fopen(fn,"w");
	if(f==NULL)
		return 1201;

// Writing averaged value
	for(i=startX;i<=finishX;i+=step)
	{
		for(j=startY;j<=finishY;j+=step)
			if(averagedValue[i*pixelsY+j]==-100)
			{
				if(!fprintf(f,"w\t"))
					return 1203;
			}
			else
				if(!fprintf(f,"%g\t",averagedValue[i*pixelsY+j]))
					return 1203;
		if(!fprintf(f,"\n"))
			return 1204;
	}
	if(makeAveragedColumns)
	{
		if(!fprintf(f,"\n"))
			return 1204;
		for(j=startY;j<=finishY;j+=step)
		{
			double sum=0;
			int count=0;
			for(i=startX;i<=finishX;i+=step)
				if(averagedValue[i*pixelsY+j]==-100)
				{
					count=0;
					break;
				}
				else
				{
					sum+=averagedValue[i*pixelsY+j];
					count++;
				}
			if(!count)
			{
				if(!fprintf(f,"w\t"))
					return 1203;
			}
			else
				if(!fprintf(f,"%g\t",sum/count))
					return 1203;
		}
		if(!fprintf(f,"\n"))
			return 1204;
	}
	if(fclose(f))
		return 1202;
	return 0;
}


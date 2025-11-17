#include <stdio.h>
#include <stdlib.h>
#include "game_of_life.h"

int main(int argc, char *argv[])
{
    int I, J;
	char filename[1000];
	FILE *file;
    cellBody *cell;
    if(argc != 2)
    {
        printf("usage: %s file_in\n", argv[0]);
        return 1;
    }
    printf("Postprocessor started\n");
    cell = malloc(cellSize);
    if (cell == NULL)
    {
        printf("Error: can't allocate memory\n");
        return 1;
    }
    printf("Loading input file: %s\n", argv[1]);
    int flag = CAT_InitPostprocessor((char *)argv[1]);
    if (flag)
    {
        printf("Error: can't read input file: %s\n", argv[1]);
        return 1;
    }

    CAT_Index index = CAT_GetArraySize();
    I = index.i;
    J = index.j;
    printf("Array size: %d x %d\nCell size = %d\n", I, J, cellSize);

    sprintf(filename, "%s.pgm", argv[1]);
	file = fopen(filename,"w");
    if (file == NULL)
    {
        printf("Error: can't write output file: %s\n", filename);
        return 1;
    }
    fprintf(file, "P2\n%d %d\n1\n", I, J);

    printf("Writing output file: %s\n", filename);
    for (int i = 0; i < I; i++)
	{
		for (int j = 0; j < J; j++)
		{
			CAT_GetCell(cell, i, j);
	        fprintf(file, "%d ", cell->Value);
	    }
		fprintf(file, "\n");
	}
	fclose(file);

	CAT_FinalizePostprocessor();
	printf("Postprocessor finished\n");
    return 0;
}

//./catmdl_game_of_life_post state.dat 
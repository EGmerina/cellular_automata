********************************************************************************

# CAT Library

Cellular Automata Topologies

********************************************************************************

### CAT_InitPreprocessor

Initialize the CAT Preprocessor environment

#### Synopsis

	int CAT_InitPreprocessor(int arrayTopology, int modelType, int
		cellSize, int globalSize, double cellsPerMeter, int arraySizeI, ...)

#### Input Parameters
	arrayTopology
		topology of the cellular array
	modelType
		type of the model
	cellSize
		size of value in a cell
	globalSize
		size of extra info
	cellsPerMeter
		number of cells per meter
	arraySizeI, ...
		sizes of array, number of sizes should be equal to dimension

#### Description
	Initiates header;
	returns  0 in case of success;
	returns -1 if error occurred when allocating file;
	returns -2 if arrayTopology is wrong;
	returns -3 if wrong number of arguments;

	Attention: number of arguments should be equal to dimension;
			if number of arguments is more than dimension, excessive arguments will not be processed;
			if number of arguments is less than dimension, not specified arguments will be equal 1.

#### Note
	Dimension is equal to arrayTopology / 10

********************************************************************************

### CAT_InitSimulator

Initialize the CAT Simulator environment

#### Synopsis

	int CAT_InitSimulator(char *filename)

#### Input paramrters
	filename
		address of file name

#### Description
	Initiates header and cellular array with information from file
	returns  0 in case of success
	returns -1 if error occurred while allocating header or cellular array
	returns -2 if error occurred while opening file
	returns -3 if error occurred while reading from file
	returns -4 if file was damaged (control sum is not the same)
	returns -5 if file name was empty

********************************************************************************

### CAT_InitPostprocessor

Initialize the CAT Postprocessor environment

#### Synopsis

	int CAT_InitPostprocessor(char *filename)

#### Input paramrters
	filename
		address of file name

#### Description
	Initiates header and cellular array with information from file
	returns  0 in case of success
	returns -1 if error occurred while allocating header or cellular array
	returns -2 if error occurred while opening file
	returns -3 if error occurred while reading from file
	returns -4 if file was damaged (control sum is not the same)
	returns -5 if file name was empty

********************************************************************************

### CAT_GetX (CAT_GetY / CAT_GetZ / CAT_GetT)



#### Synopsis

	double CAT_GetX(int i, ...)
	double CAT_GetY(int i, ...)
	double CAT_GetZ(int i, ...)
	double CAT_GetT(int i, ...)

#### Input paramrters
	i, ...
		indexes of the cell

#### Description
	Returns X (or Y, or Z, or T) coordinate of the dot representing cell
	returns -6 if topology is not implemented yet
	returns -7 if topology is not acceptable for this function
	returns -8 if header or array were not initiated
	returns -9 if indexes are out of bounds


********************************************************************************

### CAT_GetI (CAT_GetJ / CAT_GetK / CAT_GetL)


#### Synopsis

	int CAT_GetI(double x, ...)
	int CAT_GetJ(double x, ...)
	int CAT_GetK(double x, ...)
	int CAT_GetL(double x, ...)

#### Input paramrters
	x, ...
		coordinates of the dot

#### Description
	Returns i (or j, or k, or l) index of the cell representing the dot with coordinates x, ...
	returns -6 if topology is not implemented yet
	returns -7 if topology is not acceptable for this function
	returns -8 if header or array were not initiated
	returns -9 if indexes are out of bounds

********************************************************************************

### CAT_GetMaxI (CAT_GetMaxJ / CAT_GetMaxK / CAT_GetMaxL)


#### Synopsis

	int CAT_GetMaxI()
	int CAT_GetMaxJ()
	int CAT_GetMaxK()
	int CAT_GetMaxL()

#### Description
	Returns maximum i (or j, or k, or l) index.
	returns -8 if header or array were not initiated

********************************************************************************

### CAT_PutCell


#### Synopsis

	int CAT_PutCell(char *cellValue, int i, ...)

#### Input paramrters
	i, ...
		indexes of cell
	cellValue
		value to put in a cell

#### Description
	Puts value in the cell with i, ... indexes;
	returns  0 in case of success
	returns -8 if header or array were not initiated
	returns -9 if indexes are out of bounds

	Attention: number of arguments should be equal to dimension;
			if number of arguments is more than dimension, excessive arguments will not be processed;
			if number of arguments is less than dimension, not specified arguments will be equal 0.

********************************************************************************

### CAT_GetCell


#### Synopsis

	int CAT_GetCell(char *cellValue, int i, ...)

#### Input paramrters
	i, ...
		indexes of cell

#### Output variables
	cellValue
		puts there value from the cell

#### Description
	Gets value from the cell with i, ... indexes to the cellValue;
	returns  0 in case of success;
	returns -8 if header or array were not initiated
	returns -9 if indexes are out of bounds

	Attention: number of arguments should be equal to dimension;
			if number of arguments is more than dimension, excessive arguments will not be processed;
			if number of arguments is less than dimension, not specified arguments will be equal 0.

********************************************************************************

### CAT_SquareDistance

Calculates a distance between two cells

#### Synopsis

	double CAT_SquareDistance(int i1, ...)

#### Input paramrters
	i1, ...
		indexes of the first cell
	i2, ...
		indexes of the second cell

#### Description
	Returns square distance between dots representing cells with indexes i1, ... and i2, ...;
	returns  0 in case of success;
	returns -6 if topology is not implemented yet
	returns -7 if topology is not acceptable for this function
	returns -8 if header or array were not initiated
	returns -9 if indexes are out of bounds

********************************************************************************

### CAT_Iterate

#### Synopsis

	int CAT_Iterate(int (* cellTransition)(char*))

#### Input paramrters
	cellTransition
		Function used on Cellular array
		Should return 0 in case of success
#### Description
	returns 0 in case of success
	returns a non-zero number of cellTransition() was finished with error

********************************************************************************

### CAT_IsArrayChanged

#### Synopsis

	uint64_t CAT_IsArrayChanged(void)

#### Description
	returns number of changes (in bytes) were made last iteration

********************************************************************************

### CAT_GetNumThreads

#### Synopsis

	int CAT_SetNumThreads(int numThreads)

#### Input paramrters
	numThreads
		number of threads to set

#### Description
	In a parallel region
		returns the actual number of threads
	In a sequential section of the program
		returns the default number of threads to be used for subsequent parallel regions

********************************************************************************

### CAT_SetNumThreads

#### Synopsis

	int CAT_SetNumThreads(int numThreads)

#### Input paramrters
	numThreads
		number of threads to set

#### Description
	Affects the default number of threads to be used for subsequent parallel regions
	returns  0 in case of success
	returns -1 if number of threads is out of bounds

********************************************************************************

### CAT_FinalizePreprocessor

#### Synopsis

	int CAT_FinalizePreprocessor(char *filename)

#### Input paramrters
	filename
		pointer to name of file for saving caFileHeader

#### Description
	Saves caFileHeader and Cellular Array into file, frees memory.
	returns 0 in case of success
	returns 1 if error occurred during opening file
	returns 2 if error occurred during writing in the file

********************************************************************************

### CAT_FinalizeSimulator

#### Synopsis

	int CAT_FinalizeSimulator(char *filename)

#### Input paramrters
	filename
		pointer to name of file for saving caFileHeader

#### Description
	Saves caFileHeader and Cellular Array into file, frees memory.
	returns 0 in case of success
	returns 1 if error occurred during opening file
	returns 2 if error occurred during writing in the file

********************************************************************************

### CAT_FinalizePostprocessor

#### Synopsis

	int CAT_FinalizePostprocessor(char *filename)

#### Input paramrters
	filename
		pointer to name of file for saving caFileHeader

#### Description
	Saves caFileHeader and Cellular Array into file, frees memory.
	returns 0 in case of success

********************************************************************************


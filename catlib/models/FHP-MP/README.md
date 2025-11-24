********************************************************************************

# FHP-MP Model

********************************************************************************

### Compiling

	1. Make a build directory
	2. Run "cmake .. -DTOPOLOGY=HEXAGON -DOPER_MODE=SYNC -DUSE_OMP=True"
	3. Run "make"

### Preprocessor

	usage: ./catmdl_fhpmp_prep

	const char fileName[] = "initial_state.dat"; // Specifies output file
	double X = coordMax.x = 2000; // Sets X metric size
	double Y = coordMax.y = 1000; // Sets Y metric size
	const double Kl = 1.0; // Sets metric scale (cells in meter)
	int I = indexMax.i; // Gets size X of the cellular array (rows)
	int J = indexMax.j; // Gets size Y of the cellular array (columns)
	set_cell_type(i, j, CELLTYPE); // Sets a type of the cell in (i, j) position
	// where CELLTYPE is in {CONVENTIONAL, INLET, OUTLET, WALL}

### Simulator

	usage: ./catmdl_fhpmp_sim file_in file_out num_of_iters

### Postprocessor

	usage: ./catmdl_fhpmp_post file_conf
	file postprocessor.conf is an example

********************************************************************************



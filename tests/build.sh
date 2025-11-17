rm -Rf build 
mkdir build
cd build

# domino_sync
cmake .. -DTOPOLOGY=SQUARE24 -DOPER_MODE=SYNC -DMODEL_NUMBER=005
make
./test_domino_prep
./test_domino_sim
./test_domino_post
rm -Rf * 
cmake .. -DTOPOLOGY=SQUARE24 -DOPER_MODE=SYNC -DUSE_MPI=TRUE -DMODEL_NUMBER=005
make
./test_domino_prep
echo  "localhost slots=12" > hostfile
mpirun --hostfile ./hostfile -np 11 ./test_domino_sim
./test_domino_post
rm -Rf * 
cmake .. -DTOPOLOGY=SQUARE24 -DOPER_MODE=SYNC -DUSE_OMP=TRUE -DMODEL_NUMBER=005
make
./test_domino_prep
./test_domino_sim
./test_domino_post

# phase_sep
rm -Rf * 
cmake .. -DTOPOLOGY=SQUARE8 -DOPER_MODE=SYNC -DMODEL_NUMBER=001
make
./test_phase_sep_prep
./test_phase_sep_sim
./test_phase_sep_post
rm -Rf * 
cmake .. -DTOPOLOGY=SQUARE8 -DOPER_MODE=SYNC -DUSE_MPI=TRUE -DMODEL_NUMBER=001
make
./test_phase_sep_prep
echo  "localhost slots=12" > hostfile
mpirun --hostfile ./hostfile -np 11 ./test_phase_sep_sim
./test_phase_sep_post
rm -Rf * 
cmake .. -DTOPOLOGY=SQUARE8 -DOPER_MODE=SYNC -DUSE_OMP=TRUE -DMODEL_NUMBER=001
make
./test_phase_sep_prep
./test_phase_sep_sim
./test_phase_sep_post

# diff_bool_asynch
rm -Rf * 
cmake .. -DTOPOLOGY=SQUARE4 -DOPER_MODE=ASYNC -DMODEL_NUMBER=002 
make
./test_diffusion_prep
./test_diffusion_sim
./test_diffusion_post
rm -Rf * 
cmake .. -DTOPOLOGY=SQUARE4 -DOPER_MODE=ASYNC -DUSE_OMP=TRUE -DMODEL_NUMBER=002
make
./test_diffusion_prep
./test_diffusion_sim
./test_diffusion_post

# save_mass_phase_sep_async
rm -Rf *
cmake .. -DTOPOLOGY=SQUARE8 -DOPER_MODE=ASYNC -DMODEL_NUMBER=003
make
./test_phase_sep_prep
./test_phase_sep_sim
./test_phase_sep_post
rm -Rf * 
cmake .. -DTOPOLOGY=SQUARE8 -DOPER_MODE=ASYNC -DUSE_OMP=TRUE -DMODEL_NUMBER=003
make
./test_phase_sep_prep
./test_phase_sep_sim
./test_phase_sep_post

# FHP-MP
rm -Rf * 
cmake .. -DTOPOLOGY=HEXAGON -DOPER_MODE=SYNC -DMODEL_NUMBER=004
make
./test_fhpmp_prep
./test_fhpmp_sim initial_state.dat HEXAGON-OUT.dat 10
./test_fhpmp_post ../../models/FHP-MP/postprocessor.conf
rm -Rf * 
cmake .. -DTOPOLOGY=HEXAGON -DOPER_MODE=SYNC -DUSE_OMP=True -DMODEL_NUMBER=004
make
./test_fhpmp_prep
./test_fhpmp_sim initial_state.dat HEXAGON-OUT.dat 10
./test_fhpmp_post ../../models/FHP-MP/postprocessor.conf
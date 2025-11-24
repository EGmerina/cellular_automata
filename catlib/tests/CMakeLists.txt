cmake_minimum_required (VERSION 3.2.1) # [citation needed]
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp -lm -O3")


project(catlib_test)

include_directories(../src/)
add_subdirectory(../ ./catlib_build) # build catlib

link_libraries(catlib)

if(DEFINED MODEL_NUMBER)
	add_definitions(-DMODEL_NUMBER=${MODEL_NUMBER}) 
	message("Using ${MODEL_NUMBER} test")
else()
	message(SEND_ERROR "MODEL_NUMBER undefined, use 'cmake -DMODEL_NUMBER=yourTestType'")
endif()


if(${MODEL_NUMBER} STREQUAL "001")
    add_executable(test_phase_sep_prep ../models/phase_sep/phase_sep_prep.c)
    add_executable(test_phase_sep_sim ../models/phase_sep/phase_sep_sim.c)
    add_executable(test_phase_sep_post ../models/phase_sep/phase_sep_post.c)
endif()
# mpi +
# omp +

if(${MODEL_NUMBER} STREQUAL "002")
    add_executable(test_diffusion_prep ../models/test_async/test_async_prep.c)
    add_executable(test_diffusion_sim ../models/test_async/test_async_sim.c)
    add_executable(test_diffusion_post ../models/test_async/test_async_post.c)
endif()
# mpi -
# omp +

if(${MODEL_NUMBER} STREQUAL "003")
    add_executable(test_phase_sep_prep ../models/save_mass_phase_sep_async/phase_sep_prep.c)
    add_executable(test_phase_sep_sim ../models/save_mass_phase_sep_async/phase_sep_sim_async_v1.c)
    add_executable(test_phase_sep_post ../models/save_mass_phase_sep_async/phase_sep_post.c)
endif()
# mpi -
# omp +

if(${MODEL_NUMBER} STREQUAL "004")
    add_executable(test_fhpmp_prep ../models/FHP-MP/fhpmp_prep.cpp)
    add_executable(test_fhpmp_sim ../models/FHP-MP/fhpmp_sim.cpp)
    add_executable(test_fhpmp_post
    ../models/FHP-MP/postprocessor/fhpaverage.cpp
    ../models/FHP-MP/postprocessor/fhpcoord.cpp
    ../models/FHP-MP/postprocessor/fhpgraph.cpp
    ../models/FHP-MP/postprocessor/fhpio.cpp
    ../models/FHP-MP/postprocessor/fhpmisc.cpp
    ../models/FHP-MP/postprocessor/fhpphys.cpp
    ../models/FHP-MP/postprocessor/fhpvisual.cpp)
endif()
# mpi -
# omp +

if(${MODEL_NUMBER} STREQUAL "005")
    add_executable(test_domino_prep ../models/domino_sync/domino_prep.c)
    add_executable(test_domino_sim ../models/domino_sync/domino_sim.c)
    add_executable(test_domino_post ../models/domino_sync/domino_post.c)
endif()
# mpi +
# omp +
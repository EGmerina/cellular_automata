cmake_minimum_required (VERSION 3.10)# [citation needed]
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp -lm -O3")

set(MODEL_NAME seismic_waves)

project(catmdl_${MODEL_NAME})

include_directories(../../src/)
add_subdirectory(../../ ./catlib_build) # build catlib


link_libraries(catlib)

add_executable(catmdl_${MODEL_NAME}_prep ${MODEL_NAME}_prep.c)
add_executable(catmdl_${MODEL_NAME}_sim ${MODEL_NAME}_sim.c)
add_executable(catmdl_${MODEL_NAME}_post ${MODEL_NAME}_post.c)

#cmake -DTOPOLOGY=SQUARE8 -DOPER_MODE=SYNC ..
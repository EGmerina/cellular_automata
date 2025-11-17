cmake_minimum_required (VERSION 3.2.1) # [citation needed]
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive -fopenmp -lm -O3")

set(MODEL_NAME fhpmp)

project(catmdl_${MODEL_NAME})

include_directories(../../src/)
link_directories(../../src/)

add_subdirectory(../../ ./catlib_build) # build catlib

link_libraries(catlib)

add_executable(catmdl_${MODEL_NAME}_prep ${MODEL_NAME}_prep.cpp)
add_executable(catmdl_${MODEL_NAME}_sim ${MODEL_NAME}_sim.cpp)
add_executable(catmdl_${MODEL_NAME}_post
postprocessor/fhpaverage.cpp
postprocessor/fhpcoord.cpp
postprocessor/fhpgraph.cpp
postprocessor/fhpio.cpp
postprocessor/fhpmisc.cpp
postprocessor/fhpphys.cpp
postprocessor/fhpvisual.cpp)


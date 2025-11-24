cmake_minimum_required (VERSION 3.2.1) # [citation needed]

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp -lm -D_7ZIP_ST")

project(catlib)

include_directories(src/)

# TODO: are wildcards good practice?
file(GLOB catlib_src
	"src/deps/*.c"
	"src/deps/lzma/*.c"
	"src/topologies/*.c"
	# "src/*.h"
	"src/*.c"
)

find_package(MPI REQUIRED)
# link_directories($ENV{OPENMPI_LIBS})
include_directories(SYSTEM ${MPI_INCLUDE_PATH})

message(STATUS "Printing preprocessor definitions")

if(USE_MPI)
	message("Using MPI")
	add_definitions(-DUSE_MPI)
else()
	message("NOT using MPI")
endif()

if(USE_OMP)
	message("Using OMP")
	add_definitions(-DUSE_OMP)
else()
	message("NOT using OMP")
endif()

if(DEFINED TOPOLOGY)
	add_definitions(-DTOPOLOGY=${TOPOLOGY})
	message("Using ${TOPOLOGY} topology")
else()
	message(SEND_ERROR "TOPOLOGY undefined, use 'cmake -DTOPOLOGY=yourTopology'")
endif()

if(DEFINED PAR_MODE)
	add_definitions(-DPAR_MODE=${PAR_MODE})
	if(NOT (${PAR_MODE} STREQUAL "ALG1"))
		if(NOT (${PAR_MODE} STREQUAL "ALG2"))
			message(SEND_ERROR "Unknown parallellization mode ${OPER_MODE}, use either ALG1 or ALG2")
		endif()
	endif()
endif()

if(DEFINED OPER_MODE)
	add_definitions(-DOPER_MODE=${OPER_MODE})
	if(${OPER_MODE} STREQUAL "ASYNC")
		message("Using ${OPER_MODE} operating mode")
		if(DEFINED PAR_MODE)
			add_definitions(-DPAR_MODE=${PAR_MODE})
			if(USE_MPI)
				message("Using ${PAR_MODE} parallellization")
			else()
				message(WARNING "PAR_MODE defined, but MPI is not using in Asynchronous operating mode")
			endif()
		else()
			if(USE_MPI)
				message(SEND_ERROR "PAR_MODE undefined, use 'cmake -DPAR_MODE=yourParallelMode'")
			endif()
		endif()
	else()
		if(${OPER_MODE} STREQUAL "SYNC")
			message("Using ${OPER_MODE} operating mode")
			if(DEFINED PAR_MODE)
				add_definitions(-DPAR_MODE=${PAR_MODE})
				message(WARNING "Using PAR_MODE doesn't make sense in Synchronous operating mode")
			endif()
		else()
			message(SEND_ERROR "Unknown operating mode ${OPER_MODE}, use either SYNC or ASYNC")
		endif()
	endif()
else()
	message(SEND_ERROR "OPER_MODE undefined, use 'cmake -DOPER_MODE=yourOperatingMode'")
endif()

add_library(catlib STATIC ${catlib_src})

if(${USE_MPI})
	target_link_libraries(catlib ${MPI_C_LIBRARIES})
endif()

set_target_properties(catlib PROPERTIES LINKER_LANGUAGE C)
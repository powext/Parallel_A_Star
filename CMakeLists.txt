cmake_minimum_required(VERSION 3.24)

set(CMAKE_C_COMPILER "/opt/homebrew/opt/llvm/bin/clang")
set(CMAKE_CXX_COMPILER "/opt/homebrew/opt/llvm/lib/bin/clang++")

project(parallel_a_star_new C)

set(CMAKE_C_STANDARD 11)

# Paths for OpenMP
set(OPENMP_LIBRARIES "/opt/homebrew/Cellar/libomp/16.0.6/lib")
set(OPENMP_INCLUDES "/opt/homebrew/Cellar/libomp/16.0.6/include")

# Find OpenMP
if(APPLE AND CMAKE_C_COMPILER_ID MATCHES "Clang")
    set(OpenMP_C_FLAGS "-fopenmp=libomp -Wno-unused-command-line-argument")
    set(OpenMP_CXX_FLAGS "-fopenmp=libomp -Wno-unused-command-line-argument")
    link_directories("${OPENMP_LIBRARIES}")
endif()

find_package(MPI REQUIRED)
find_package(OpenMP REQUIRED)

include_directories("${OPENMP_INCLUDES}")

add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

add_executable(parallel_a_star_new
        include/main.h
        src/main.c
        include/adjlist.h
        src/adjlist.c
        include/cJSON.h
        src/cJSON.c
        include/comm.h
        src/comm.c
        include/compute_distance.h
        src/compute_distance.c
        include/compute_path.h
        src/compute_path.c
        include/exit_points.h
        src/exit_points.c
        include/json_output.h
        src/json_output.c
        include/parallel.h
        src/parallel.c
        include/parallel_collection.h
        src/parallel_collection.c
        include/parallel_distribution.h
        src/parallel_distribution.c
        include/parallel_paths.h
        src/parallel_paths.c
        include/colors.h
        include/print.h
        src/print.c
        include/priority_queue.h
        src/priority_queue.c
        include/utility.h
        src/utility.c
        # src/input_generator.c
)

target_link_libraries(parallel_a_star_new PRIVATE MPI::MPI_C)
target_link_libraries(parallel_a_star_new PRIVATE "${OPENMP_LIBRARIES}/libomp.dylib")

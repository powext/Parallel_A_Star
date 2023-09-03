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

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS} -fsanitize=address -g")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

add_executable(parallel_a_star_new src/main.c src/compute_distance.c src/hash_table.c src/input_generator.c src/generic_list.c src/priority_queue.c src/utility.c include/compute_distance.h include/main.h include/colors.h include/print.h src/print.c include/parallel.h src/parallel.c
        src/comm.c
        include/comm.h
        src/compute_path.c
        include/util.h)

target_link_libraries(parallel_a_star_new PRIVATE MPI::MPI_C)
target_link_libraries(parallel_a_star_new PRIVATE "${OPENMP_LIBRARIES}/libomp.dylib")


target_link_libraries(parallel_a_star_new PRIVATE "${OPENMP_LIBRARIES}/libomp.dylib")

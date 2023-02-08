cmake_minimum_required(VERSION 3.24)
project(parallel_a_star_new C)

set(CMAKE_C_STANDARD 11)

add_executable(parallel_a_star_new src/main.c src/compute_distance.c src/hash_table.c src/input_generator.c src/generic_list.c src/priority_queue.c src/utility.c include/compute_distance.h include/main.h)

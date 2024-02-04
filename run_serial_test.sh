#!/bin/bash

module load mpich-3.2
# Set OMP_NUM_THREADS environment variable
export OMP_NUM_THREADS=1

# Execute the mpiexec command
mpirun.actual -n 1 ./Parallel_A_Star/parallel_a_star -file $FILE

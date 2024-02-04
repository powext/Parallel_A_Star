#!/bin/bash

module load mpich-3.2
# Set OMP_NUM_THREADS environment variable
export OMP_NUM_THREADS=$THREAD

# Execute the mpiexec command
mpirun.actual -n $PROCESS ./Parallel_A_Star/parallel_a_star -file $FILE -parallel

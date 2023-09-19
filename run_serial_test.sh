#!/bin/bash

NODES=1
CPUS=1
MEM_GB=10
MAX_MINUTES=10
FILE=$1
DIMENSION=$2
ITERATION=$3

#PBS -l select=$NODES:ncpus=$CPUS:mem=${MEM_GB}gb
#PBS -l walltime=0:$MAX_MINUTES:00
#PBS -q short_cpuQ

#PBS -e ~error_test_${DIMENSION}_${ITERATION}_serial.log
#PBS -o ~test_${DIMENSION}_${ITERATION}_serial.log

# Set OMP_NUM_THREADS environment variable
export OMP_NUM_THREADS=1

# Execute the mpiexec command
mpiexec -np 1 ./cmake-build-debug/parallel_a_star_new -file "$FILE"


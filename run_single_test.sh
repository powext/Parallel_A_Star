#!/bin/bash

NODES=$1
CPUS=$2
MEM_GB=10
MAX_MINUTES=10
FILE=$3
DIMENSION=$4
PROCESSES=$5
THREADS=$6
ITERATION=$7

#PBS -l select=$NODES:ncpus=$CPUS:mem=${MEM_GB}gb
#PBS -l walltime=0:$MAX_MINUTES:00
#PBS -q short_cpuQ

#PBS -e ~error_test_${DIMENSION}_${ITERATION}_${NODES}_${CPUS}_${PROCESSES}_${THREADS}.log
#PBS -o ~test_${DIMENSION}_${ITERATION}_${NODES}_${CPUS}_${PROCESSES}_${THREADS}.log

module load mpich-3.2
module load omp

# Set OMP_NUM_THREADS environment variable
export OMP_NUM_THREADS="$THREADS"

# Execute the mpiexec command
mpiexec -np "$PROCESSES" ./cmake-build-debug/parallel_a_star_new -file "$FILE" -parallel


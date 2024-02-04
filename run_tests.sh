#!/bin/bash

echo "Starting tests!!!"

MEM_GB=4
MAX_MINUTES=10

# Define the directory containing the files
DIRECTORY="data"

# Create or clear the "results" file
OUTPUT_FILE="test_results.txt"
> $OUTPUT_FILE

NODES=(1 2)
CPUS=(1 2 4 8)
OMP_NUM_THREADS_VALUES=(1 2 4 8)
PROCESS_NUM_VALUES=(1 4 16)

index=0
# Iterate through files in the "data" directory
for file in "$DIRECTORY"/*; do
  # Check if the file is valid (exists and is a regular file)
  if [ -f "$file" ]; then
    if [[ $file =~ ([0-9]+)([^_]+)$ ]]; then
      dimension="${BASH_REMATCH[1]}"
    fi

    # Loop through each value of nodes
    for nodes_num in "${NODES[@]}"; do
      # Loop through each value of cpus
      for cpu_num in "${CPUS[@]}"; do
        
        if [ $nodes_num -gt $cpu_num ]; then
          continue
        fi
        
        # Loop through each value of process_num
        for process_num in "${PROCESS_NUM_VALUES[@]}"; do
          # Loop through each value of threads num
          for omp_threads in "${OMP_NUM_THREADS_VALUES[@]}"; do

            NODE=$nodes_num
            CPU=$cpu_num
            FILE="./Parallel_A_Star/$file"
            DIMENSION=$dimension
            PROCESS=$process_num
            THREAD=$omp_threads
            
            echo "$FILE, PARALLEL, DIMENSION=$dimension, NODES=$nodes_num, CPUS=$cpu_num, PROCESS_NUM=$process_num, OMP_NUM_THREADS=$omp_threads"  >> "$OUTPUT_FILE"
            
            # Run the script 5 times
            for i in {1..5}; do
              ITERATION=$i

              N_JOBS=$(qstat | grep jacopo.clocchiat | grep common | grep "^.*$" -c)
              while [[ $N_JOBS -ge 29 ]]; do
                  # echo "max number of jobs submitted, waiting for some to finish"
                  N_JOBS=$(qstat | grep jacopo.clocchiat | grep common | grep "^.*$" -c)
                  
                  # if [[ $N_JOBS -gt  ]]; then
                  #   N_JOBS=30
                  
                  # fi
              done
              qsub -l nodes=${NODE},ncpus=${CPU},mem=${MEM_GB}gb,walltime=10:00:00 -q common_cpuQ -v THREAD=$THREAD,PROCESS=$PROCESS,FILE=$FILE -e ~/Parallel_A_Star/error_logs/error_${DIMENSION}_${ITERATION}_${NODE}_${CPU}_${PROCESS}_${THREAD}.log -o ~/Parallel_A_Star/logs/results_${DIMENSION}_${ITERATION}_${NODE}_${CPU}_${PROCESS}_${THREAD}.log run_single_test.sh >> "$OUTPUT_FILE"
            done
            echo "DONE PARALLEL, DIMENSION=$dimension, NODES=$nodes_num, CPUS=$cpu_num, PROCESS_NUM=$process_num, OMP_NUM_THREADS=$omp_threads"
          done
        done
      done
    done
  fi
done

for file in "$DIRECTORY"/*; do
  # Check if the file is valid (exists and is a regular file)
  if [ -f "$file" ]; then
    if [[ $file =~ ([0-9]+)([^_]+)$ ]]; then
      dimension="${BASH_REMATCH[1]}"
    fi
    NODE=1
    CPU=1
    FILE=$file
    DIMENSION=$dimension
    PROCESS=1
    THREAD=1
    echo "SERIAL, DIMENSION=$dimension" >> $OUTPUT_FILE
    for i in {1..5}; do
      ITERATION=$i
      N_JOBS=$(qstat | grep jacopo.clocchiat | grep "^.*$" -c)
      while [[ $N_JOBS -ge 29 ]]; do
          # echo "max number of jobs submitted, waiting for some to finish"
          N_JOBS=$(qstat | grep jacopo.clocchiat | grep "^.*$" -c)
      done
      qsub -l nodes=${NODE},ncpus=${CPU},mem=${MEM_GB}gb,walltime=99:00:00 -q common_cpuQ -v FILE=$FILE -e ~/Parallel_A_Star/error_logs/error_test_${DIMENSION}_${ITERATION}_serial.log -o ~/Parallel_A_Star/logs/test_${DIMENSION}_${ITERATION}_serial.log run_serial_test.sh >> "$OUTPUT_FILE"
    done
  fi
done


# 10419
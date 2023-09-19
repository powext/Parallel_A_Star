#!/bin/bash

MEM_GB=4
MAX_MINUTES=10
# Define the directory containing the files
DIRECTORY="data"

# Create or clear the "results" file
OUTPUT_FILE="results.txt"
> $OUTPUT_FILE

NODES=(1 2 4 8)
CPUS=(1 2 4 8)
OMP_NUM_THREADS_VALUES=(1 2 4 8)
PROCESS_NUM_VALUES=(1 4 9 16)

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
        # Loop through each value of process_num
        for process_num in "${PROCESS_NUM_VALUES[@]}"; do
          # Loop through each value of threads num
          for omp_threads in "${OMP_NUM_THREADS_VALUES[@]}"; do
            echo "PARALLEL, DIMENSION=$dimension, NODES=$nodes_num, CPUS=$cpu_num, PROCESS_NUM=$process_num, OMP_NUM_THREADS=$omp_threads"  >> "$OUTPUT_FILE"

            # Run the script 5 times
            for i in {1..5}; do
              qsub ./run_single_test.sh "$nodes_num" "$cpu_num" "$file" "$dimension" "$process_num" "$omp_threads" "$i" >> "$OUTPUT_FILE"
            done
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
    echo "SERIAL, DIMENSION=$dimension" >> $OUTPUT_FILE
    for i in {1..5}; do
      qsub ./run_serial_test.sh "$file" "$dimension" "$i" >> "$OUTPUT_FILE"
    done

  fi
done
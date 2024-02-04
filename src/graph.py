import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

if __name__ == "__main__":
    # Read the CSV file (replace 'your_csv_file.csv' with the actual file path)
    csv_file = '/Users/jacopoclocchiatti/Documents/Uni/UniTn-local/HPC/Parallel_A_Star/results.csv'
    df = pd.read_csv(csv_file, header=0, dtype={'dimension_of_sample': np.int32,
                                                'elapsed_time': np.float64,
                                                'unit_of_measure': str})
    df = df.sort_values(by='dimension_of_sample')
    print(df.tail())
    # Extract the data columns
    dimension_of_sample = df['dimension_of_sample']
    elapsed_time = df['elapsed_time']

    # Create the line plot
    plt.figure(figsize=(10, 6))
    plt.plot(dimension_of_sample, elapsed_time, marker='o', color='blue', linestyle='-')
    plt.title('Elapsed Time vs Dimension of Sample')
    plt.xlabel('Dimension of Sample')
    plt.ylabel('Elapsed Time (unit_of_measure)')
    plt.grid(True)

    # Show the plot
    plt.show()
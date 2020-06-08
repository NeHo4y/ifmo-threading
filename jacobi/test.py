import csv
import time
import numpy as np
import subprocess
import matplotlib.pyplot as plot

EXECUTABLE = "./cpp/cmake-build-release/jacobi"

processes_counts = (1, 2, 4, 6)
system_ranks = (100, 200, 500, 1000, 2000, 5000)
precision = 1e-3

iterations = 100

benchmarks = {
    processes_count: {size: None for size in system_ranks}
    for processes_count in processes_counts
}


def benchmark():
    for size in system_ranks:
        benchmark_system(size)


def benchmark_system(system_rank):
    print(f"Benchmarking system of rank {system_rank}")
    system_params = generate_system(system_rank)
    with open("linear.input", "w") as file:
        file.write(f"{system_rank} {precision / 10}\n")
        for row in system_params:
            file.write(" ".join(str(e) for e in row) + "\n")
    for processes_count in processes_counts:
        duration = benchmark_system_with_processes_count(processes_count)
        benchmarks[processes_count][system_rank] = duration
        verify_system_solved(system_params[2:], system_params[1])


def generate(system_rank):
    A = np.random.rand(system_rank, system_rank) * 10 - 5
    X = np.random.rand(1, system_rank)
    B = np.random.rand(1, system_rank) * 10 - 5
    for row in range(system_rank):
        A[row][row] = np.sum(np.abs(A[row])) - np.abs(A[row][row]) + np.random.rand() * 2 + 2
    q = np.linalg.norm(B)
    print(q)
    return np.concatenate((X, B, A)), A, B.flatten()


def generate_system(rank):
    while True:
        parameters = np.random.rand(rank + 2, rank)
        A = parameters[2:]

        # Enforce diagonal
        A = A + np.identity(rank) * A * 100000

        E = np.eye(rank)
        D = np.identity(rank) * A
        B = E - np.dot(np.linalg.inv(D), A)
        q = np.linalg.norm(B)
        if q < 0.5:
            break
    parameters[2:] = A
    print(f"q = {q}")
    return parameters, parameters[2:], parameters[1]


def verify_system_solved(A, b):
    x = np.expand_dims(np.fromfile("linear.output", sep=" "), axis=1)
    ax = np.dot(A, x)
    ax = ax.flatten()
    correct = all(abs(ax - b) <= precision)
    if not correct:
        raise Exception("Solution incorrect")


def benchmark_system_with_processes_count(processes_count) -> float:
    print(f"Benchmarking {processes_count} processes")

    start = time.time()
    for i in range(iterations):
        exit_code = subprocess.call(["mpiexec", "-n", str(processes_count), EXECUTABLE])
        assert exit_code == 0
    finish = time.time()
    duration = (finish - start) / iterations
    print("Duration: %.2f seconds" % duration)
    return duration


def plot_results():
    serial_durations = tuple(benchmarks[1].values())
    for processes_count in benchmarks:
        durations = tuple(benchmarks[processes_count].values())
        accelerations = tuple(sd / d for sd, d in zip(serial_durations, durations))
        plot.plot(system_ranks, accelerations, marker='o')
    plot.legend(processes_counts)
    plot.show()


def save_results():
    records = []
    for process_count in benchmarks:
        for system_size in benchmarks[process_count]:
            records.append((system_size, process_count, benchmarks[process_count][system_size]))
    records.sort(key=lambda record: record[0])
    with open("results.csv", "w") as file:
        writer = csv.writer(file)
        writer.writerow(["length", "processes", "time"])
        for record in records:
            writer.writerow(record)
    print(records)


if __name__ == '__main__':
    np.random.seed(1337)
    benchmark()
    plot_results()
    save_results()

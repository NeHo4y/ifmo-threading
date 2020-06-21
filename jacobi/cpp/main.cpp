#include <mpi.h>
#include "Jacobi.cpp"
#include <chrono>
#include "iostream"

using namespace std::chrono;

int main() {
    MPI_Init(nullptr, nullptr);

    Jacobi solver;
    solver.process_params();
    auto start = high_resolution_clock::now();
    solver.init();
    int iteration = 0;

    for (iteration = 0; iteration < 1000 && !solver.precisionReached(); iteration++) {
        solver.computeIteration();
    }
    auto finish = high_resolution_clock::now();
    cout << duration<double>(finish - start).count() << endl;
    solver.outputResult();
    MPI_Finalize();
    return solver.precisionReached() ? 0 : -1;
}

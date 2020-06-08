#include <mpi.h>
#include "Jacobi.cpp"

int main() {
    MPI_Init(nullptr, nullptr);

    Jacobi solver;
    solver.init();
    for (int iteration = 0; iteration < 1000 && !solver.precisionReached(); iteration++) {
        solver.computeIteration();
    }
    solver.outputResult();

    MPI_Finalize();
    return solver.precisionReached() ? 0 : -1;
}

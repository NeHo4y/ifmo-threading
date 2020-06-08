#pragma once

#include <fstream>

using namespace std;

const int PRIMARY_PROCESS = 0;

class Jacobi {
private:

    double *A;        // coefficient matrix. converted to 1 dim for bcast
    double *b;        // matrix [n, 1] here reduced dim to [n]
    double *x;        // matrix [n, 1] here as [n]

    int systemRank;
    double precision; // required precision

    double *xPrev = nullptr;   // previous x

    // MPI process communication specific, batching
    int currentProcess;
    int totalProcesses;

    // this process will calculate a part [offset; offset + size] of array x
    int offset;
    int batchSize;

public:
    Jacobi() {
        MPI_Comm_rank(MPI_COMM_WORLD, &currentProcess);
        MPI_Comm_size(MPI_COMM_WORLD, &totalProcesses);
    }

    void init() {
        if (currentProcess == PRIMARY_PROCESS) {
            readParameters();
        }

        MPI_Bcast(&systemRank, 1, MPI_INT, PRIMARY_PROCESS, MPI_COMM_WORLD);
        MPI_Bcast(&precision, 1, MPI_DOUBLE, PRIMARY_PROCESS, MPI_COMM_WORLD);
        if (currentProcess != PRIMARY_PROCESS) {
            A = new double[systemRank * systemRank];
            b = new double[systemRank];
            x = new double[systemRank];
        }

        MPI_Bcast(A, systemRank * systemRank, MPI_DOUBLE, PRIMARY_PROCESS, MPI_COMM_WORLD);
        MPI_Bcast(b, systemRank, MPI_DOUBLE, PRIMARY_PROCESS, MPI_COMM_WORLD);
        MPI_Bcast(x, systemRank, MPI_DOUBLE, PRIMARY_PROCESS, MPI_COMM_WORLD);

        if (currentProcess == PRIMARY_PROCESS) {
            const int partSize = systemRank / totalProcesses;
            for (int process = 0; process < totalProcesses; process++) {
                int processOffset = process * partSize;
                int processBatchSize = (process < totalProcesses - 1) ? partSize : systemRank - processOffset;
                if (process == PRIMARY_PROCESS) {
                    this->offset = processOffset;
                    this->batchSize = processBatchSize;
                } else {
                    MPI_Send(&processOffset, 1, MPI_INT, process, 0, MPI_COMM_WORLD);
                    MPI_Send(&processBatchSize, 1, MPI_INT, process, 0, MPI_COMM_WORLD);
                }
            }
        } else {
            MPI_Status status;
            MPI_Recv(&offset, 1, MPI_INT, PRIMARY_PROCESS, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            MPI_Recv(&batchSize, 1, MPI_INT, PRIMARY_PROCESS, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        }
    }

    void computeIteration() {
        updatePrevious();
        computeStep();
        broadcastChunk();
    }

    bool precisionReached() {
        if (xPrev == nullptr) {
            return false;
        }
        for (int i = 0; i < systemRank; i++) {
            if (abs(x[i] - xPrev[i]) > precision) {
                return false;
            }
        }
        return true;
    }

    void outputResult() {
        if (currentProcess == PRIMARY_PROCESS) {
            ofstream stream("linear.output");
            for (auto i = 0; i < systemRank; i++) {
                stream << x[i] << " ";
            }
            stream.close();
        }
    }

private:
    void readParameters() {
        ifstream stream("linear.input");
        stream >> systemRank >> precision;
        x = new double[systemRank];
        A = new double[systemRank * systemRank];
        b = new double[systemRank];
        for (auto i = 0; i < systemRank; i++) {
            stream >> x[i];
        }
        for (auto i = 0; i < systemRank; i++) {
            stream >> b[i];
        }
        for (auto i = 0; i < systemRank; i++) {
            for (auto j = 0; j < systemRank; j++) {
                stream >> A[i * systemRank + j];
            }
        }
        stream.close();
    }

    void updatePrevious() {
        if (xPrev == nullptr) {
            xPrev = new double[systemRank];
        }
        for (int i = 0; i < systemRank; i++) {
            xPrev[i] = x[i];
        }
    }

    void computeStep() {
        for (int i = offset; i < offset + batchSize; i++) {
            double sum = 0;
            double diag = 0;
            for (int j = 0; j < systemRank; j++) {
                if (i != j) {
                    sum += A[i * systemRank + j] * xPrev[j];
                } else {
                    diag = A[i * systemRank + j];
                }
            }
            x[i] = (b[i] - sum) / diag;
        }
    }

    void broadcastChunk() {
        MPI_Status status;
        const int partSize = systemRank / totalProcesses;
        for (int process = 0; process < totalProcesses; process++) {
            const int offset = process * partSize;
            const int batchSize = (process < totalProcesses - 1) ? partSize : systemRank - offset;
            MPI_Bcast(&x[offset], batchSize, MPI_DOUBLE, process, MPI_COMM_WORLD);
        }
    }
};

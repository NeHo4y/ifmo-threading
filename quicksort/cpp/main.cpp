#include <fstream>
#include <mpi.h>
#include "sorting.cpp"
#include <chrono>

using namespace std;
using namespace std::chrono;

splittableArray *readArray(int pivotType);

void writeArray(splittableArray *array);

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    int pivotType = atoi(argv[1]);
    Sorter quicksort(pivotType);

    if (quicksort.globalGroup->currentProcess == PRIMARY_PROCESS) {
        quicksort.initialize(readArray(pivotType));
    } else {
        quicksort.initialize(nullptr);
    }

    auto start = high_resolution_clock::now();
    while (true) {
        if (quicksort.group->totalProcesses == 1) {
            quicksort.sort();
            break;
        }
        quicksort.pivot();
        quicksort.exchange();
        quicksort.regroup();
    }
    splittableArray *array;
    if (quicksort.globalGroup->currentProcess == PRIMARY_PROCESS) {
        array = quicksort.collect();
    } else {
        quicksort.collect();
    }

    auto finish = high_resolution_clock::now();
    cout << duration<double>(finish - start).count() << endl;
    if (quicksort.globalGroup->currentProcess == PRIMARY_PROCESS) {
        writeArray(array);
    }

    MPI_Finalize();
    return 0;
}

splittableArray *readArray(int pivotType) {
    ifstream stream("quicksort.input");
    int *content, size;
    stream >> size;
    content = new int[size];
    for (auto i = 0; i < size; i++) {
        stream >> content[i];
    }
    stream.close();
    return new splittableArray(content, size, pivotType);
}

void writeArray(splittableArray *array) {
    ofstream stream("quicksort.output");
    stream << array->size << endl;
    for (auto i = 0; i < array->size; i++) {
        stream << array->content[i] << " ";
    }
    stream.close();
}

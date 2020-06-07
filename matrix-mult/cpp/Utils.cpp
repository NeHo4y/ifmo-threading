#include "Matrix.cpp"

void generateAndPersistMatrices(int height, int width) {
    auto b = 500;
    Matrix(height, b).writeToFile("A.matrix");
    Matrix(b, width).writeToFile("B.matrix");
}

void fetchMatrices(Matrix **A, Matrix **B) {
    *A = new Matrix("A.matrix");
    *B = new Matrix("B.matrix");
}

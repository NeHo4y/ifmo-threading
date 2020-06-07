#pragma once

#include <iostream>
#include <fstream>

using namespace std;

class Matrix {
public:
    int height;
    int width;
    int **matrixValues;

    Matrix() : Matrix(0, 0) {}

    static int randomInt(int leftBound, int rightBound) {
        return leftBound + rand() % (rightBound - leftBound);
    }

    Matrix(int height, int width) : height(height), width(width) {
        matrixValues = new int *[height];
        for (auto row = 0; row < height; row++) {
            matrixValues[row] = new int[width];
            for (auto column = 0; column < width; column++) {
                matrixValues[row][column] = randomInt(-1000, 1000);
            }
        }
    }

    Matrix(string filename) {
        ifstream stream(filename);
        stream >> height >> width;
        matrixValues = new int *[height];
        for (auto row = 0; row < height; row++) {
            matrixValues[row] = new int[width];
            for (auto column = 0; column < width; column++) {
                stream >> matrixValues[row][column];
            }
        }
        stream.close();
    }

    void writeToFile(string filename) {
        ofstream stream(filename);
        stream << height << " " << width << endl;
        for (auto row = 0; row < height; row++) {
            for (auto column = 0; column < width; column++) {
                stream << matrixValues[row][column] << " ";
            }
            stream << endl;
        }
        stream.close();
    }
};

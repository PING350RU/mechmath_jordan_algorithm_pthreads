#pragma once

#include <vector>

using Matrix = std::vector<std::vector<double>>;

double formula(int k, int n, int i, int j);
bool initMatrix(int n, int k, const char* filename, Matrix& a);
int jordanInverse(int n, Matrix& a, Matrix& inverse, int threads);
void printMatrix(const Matrix& a, int rows, int cols, int m);
double residualNorm(const Matrix& a, const Matrix& inverse);

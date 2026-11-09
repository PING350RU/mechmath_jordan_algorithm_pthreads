#include "functions.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <string>

using namespace std;

bool readInt(const char* text, int& value) {
    try {
        size_t pos = 0;
        value = stoi(text, &pos);
        return text[pos] == '\0';
    } catch (...) {
        return false;
    }
}

void printMatrix(const Matrix& a, int rows, int cols, int m) {
    for (int i = 0; i < min(rows, m); i++) {
        for (int j = 0; j < min(cols, m); j++) {
            printf(" %10.3e", a[i][j]);
        }
        printf("\n");
    }
}

double residualNorm(const Matrix& a, const Matrix& inverse) {
    int n = static_cast<int>(a.size());
    vector<double> row(n);
    double result = 0.0;

    for (int i = 0; i < n; i++) {
        fill(row.begin(), row.end(), 0.0);

        for (int k = 0; k < n; k++) {
            for (int j = 0; j < n; j++) {
                row[j] += a[i][k] * inverse[k][j];
            }
        }

        for (int j = 0; j < n; j++) {
            double diff = row[j] - (i == j ? 1.0 : 0.0);
            result += diff * diff;
        }
    }

    return sqrt(result);
}

int main(int argc, char* argv[]) {
    int n, m, k, threads;

    if (argc < 5 || argc > 6 ||
        !readInt(argv[1], n) || !readInt(argv[2], m) ||
        !readInt(argv[3], k) || !readInt(argv[4], threads) ||
        n <= 0 || m <= 0 || k < 0 || k > 4 ||
        threads < 1 ||
        (k == 0 && argc != 6) || (k != 0 && argc != 5)) {
        cerr << "Использование: jordan n m k threads [filename], k = 0..4\n";
        return 1;
    }

    const char* filename = k == 0 ? argv[5] : nullptr;
    Matrix a(n, vector<double>(n, 0.0));
    Matrix inverse(n, vector<double>(n, 0.0));

    if (!initMatrix(n, k, filename, a)) {
        cerr << "!Невозможно прочитать матрицу!";
        return 1;
    }

    printf("Матрица A:\n");
    printMatrix(a, n, n, m);

    auto start = chrono::steady_clock::now();
    int status = jordanInverse(n, a, inverse, threads);
    auto finish = chrono::steady_clock::now();
    double seconds = chrono::duration<double>(finish - start).count();

    if (status == 1) {
        cerr << "!Матрица вырождена!";
        return 2;
    }
    if (status == 2) {
        cerr << "!Невозможно создать нить!";
        return 3;
    }

    printf("Обратная матрицу:\n");
    printMatrix(inverse, n, n, m);

    if (!initMatrix(n, k, filename, a)) {
        cerr << "!Невозможно прочитать матрицу!\n";
        return 1;
    }

    printf("Остаток: %.3e\n", residualNorm(a, inverse));
    printf("Время: %.6f s\n", seconds);
    printf("Нити: %d\n", threads);
    return 0;
}

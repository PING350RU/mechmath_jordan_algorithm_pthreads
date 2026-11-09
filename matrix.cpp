#include "functions.h"

#include <algorithm>
#include <cmath>
#include <fstream>

using namespace std;

double formula(int k, int n, int i, int j) {
    if (k == 1) return n - max(i, j) + 1;
    if (k == 2) return max(i, j);
    if (k == 3) return abs(i - j);
    return 1.0 / (i + j - 1);
}

bool initMatrix(int n, int k, const char* filename, Matrix& a) {
    if (k != 0) {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                a[i][j] = formula(k, n, i + 1, j + 1);
            }
        }
        return true;
    }

    ifstream file(filename);
    if (!file) return false;

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (!(file >> a[i][j]) || !isfinite(a[i][j])) return false;
        }
    }

    char extra;
    if (file >> extra) return false;
    return true;
}

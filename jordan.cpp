#include "functions.h"

#include <algorithm>
#include <cmath>
#include <pthread.h>
#include <vector>

using namespace std;

struct Shared {
    int n;
    int threads;
    int col = 0;
    int generation = 0;
    int finished = 0;
    bool stop = false;
    Matrix* a;
    Matrix* inverse;
    pthread_mutex_t mutex;
    pthread_cond_t start;
    pthread_cond_t done;
};

struct WorkerArgs {
    Shared* shared;
    int id;
};

void eliminateRows(Shared& s, int id, int col) {
    Matrix& a = *s.a;
    Matrix& inverse = *s.inverse;

    for (int row = id; row < s.n; row += s.threads) {
        if (row == col) continue;

        double factor = a[row][col];

        for (int j = col + 1; j < s.n; j++) {
            a[row][j] -= factor * a[col][j];
        }
        a[row][col] = 0.0;

        for (int j = 0; j < s.n; j++) {
            inverse[row][j] -= factor * inverse[col][j];
        }
    }
}

void* worker(void* pointer) {
    WorkerArgs* arg = static_cast<WorkerArgs*>(pointer);
    Shared& s = *arg->shared;
    int seen = 0;

    while (true) {
        pthread_mutex_lock(&s.mutex);
        while (!s.stop && seen == s.generation) {
            pthread_cond_wait(&s.start, &s.mutex);
        }
        if (s.stop) {
            pthread_mutex_unlock(&s.mutex);
            break;
        }
        seen = s.generation;
        int col = s.col;
        pthread_mutex_unlock(&s.mutex);

        eliminateRows(s, arg->id, col);

        pthread_mutex_lock(&s.mutex);
        s.finished++;
        if (s.finished == s.threads) {
            pthread_cond_signal(&s.done);
        }
        pthread_mutex_unlock(&s.mutex);
    }

    return nullptr;
}

int jordanInverse(int n, Matrix& a, Matrix& inverse, int threads) {
    for (int i = 0; i < n; i++) {
        inverse[i][i] = 1.0;
    }

    Shared s;
    s.n = n;
    s.threads = threads;
    s.a = &a;
    s.inverse = &inverse;

    vector<pthread_t> workers;
    vector<WorkerArgs> args;

    if (threads > 1) {
        pthread_mutex_init(&s.mutex, nullptr);
        pthread_cond_init(&s.start, nullptr);
        pthread_cond_init(&s.done, nullptr);
        workers.resize(threads);
        args.resize(threads);

        for (int id = 0; id < threads; id++) {
            args[id] = {&s, id};
            if (pthread_create(&workers[id], nullptr, worker, &args[id])) {
                pthread_mutex_lock(&s.mutex);
                s.stop = true;
                pthread_cond_broadcast(&s.start);
                pthread_mutex_unlock(&s.mutex);
                for (int j = 0; j < id; j++) pthread_join(workers[j], nullptr);
                pthread_cond_destroy(&s.done);
                pthread_cond_destroy(&s.start);
                pthread_mutex_destroy(&s.mutex);
                return 2;
            }
        }
    }

    const double eps = 1e-15;
    int status = 0;

    for (int col = 0; col < n; col++) {
        int mainRow = col;

        for (int row = col + 1; row < n; row++) {
            if (abs(a[row][col]) > abs(a[mainRow][col])) {
                mainRow = row;
            }
        }

        if (abs(a[mainRow][col]) < eps) {
            status = 1;
            break;
        }

        swap(a[col], a[mainRow]);
        swap(inverse[col], inverse[mainRow]);

        double pivot = a[col][col];

        for (int j = col + 1; j < n; j++) {
            a[col][j] /= pivot;
        }
        a[col][col] = 1.0;

        for (int j = 0; j < n; j++) {
            inverse[col][j] /= pivot;
        }

        if (threads == 1) {
            eliminateRows(s, 0, col);
        } else {
            pthread_mutex_lock(&s.mutex);
            s.col = col;
            s.finished = 0;
            s.generation++;
            pthread_cond_broadcast(&s.start);
            while (s.finished < threads) {
                pthread_cond_wait(&s.done, &s.mutex);
            }
            pthread_mutex_unlock(&s.mutex);
        }
    }

    if (threads > 1) {
        pthread_mutex_lock(&s.mutex);
        s.stop = true;
        pthread_cond_broadcast(&s.start);
        pthread_mutex_unlock(&s.mutex);

        for (int id = 0; id < threads; id++) {
            pthread_join(workers[id], nullptr);
        }
        pthread_cond_destroy(&s.done);
        pthread_cond_destroy(&s.start);
        pthread_mutex_destroy(&s.mutex);
    }

    return status;
}

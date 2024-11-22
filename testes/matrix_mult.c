#include "paralelismo/paralelismo.h"

#define SIZE 1000
#define THREADS 4

int A[SIZE][SIZE], B[SIZE][SIZE], C[SIZE][SIZE];
int step = 0;
int foo = 1;

typedef struct data {
    int start;
    int end;
} Data;

void multiply(Data *d) {
    for (int i = d->start; i < d->end; i++) {
         for (int j = 0; j < SIZE; j++) {
            int sum = 0;
            for (int k = 0; k < SIZE; k++) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
         }
    }
}

int main() {
    WaitGroup wg;
    cria_wait_group(&wg);
    // Initialize matrices A and B
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            A[i][j] = foo++;
            B[i][j] = foo++;
        }
    }

    int chunk = SIZE / THREADS;
    Data d[THREADS];
    for (int i = 0; i < THREADS; i++) {
        d[i].start = i * chunk;
        d[i].end = (i + 1) * chunk;
    }
    for (int i = 0; i < THREADS; i++) {
        cria_thread(multiply, &d[i], &wg);
    }
    wait(&wg);
    return 0;
}


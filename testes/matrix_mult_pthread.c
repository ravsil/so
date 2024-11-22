#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define SIZE 1000
#define THREADS 4

int A[SIZE][SIZE], B[SIZE][SIZE], C[SIZE][SIZE];
int foo = 1;
int step = 0;

void* multiply(void* arg) {
    int thread_part = step++;
    for (int i = thread_part * SIZE / THREADS; i < (thread_part + 1) * SIZE / THREADS; i++) {
        for (int j = 0; j < SIZE; j++) {
            C[i][j] = 0;
            for (int k = 0; k < SIZE; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    return NULL;
}

int main() {
    // Initialize matrices A and B
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            A[i][j] = foo++;
            B[i][j] = foo++;
        }
    }

    pthread_t threads[THREADS];

    // Create threads
    for (int i = 0; i < THREADS; i++) {
        pthread_create(&threads[i], NULL, multiply, NULL);
    }

    // Wait for threads to complete
    for (int i = 0; i < THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Print result matrix C (optional)
    /*
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            printf("%d ", C[i][j]);
        }
        printf("\n");
    }
    */

    return 0;
}


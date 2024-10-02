#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MATRIX_SIZE 500
#define FIB_N 40
#define PRIME_LIMIT 100000

// Função para multiplicação de matrizes
void matrix_multiply() {
    int matA[MATRIX_SIZE][MATRIX_SIZE], matB[MATRIX_SIZE][MATRIX_SIZE], result[MATRIX_SIZE][MATRIX_SIZE];

    // Inicializa matrizes com valores arbitrários
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matA[i][j] = rand() % 10;
            matB[i][j] = rand() % 10;
        }
    }

    // Multiplicação das matrizes
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            result[i][j] = 0;
            for (int k = 0; k < MATRIX_SIZE; k++) {
                result[i][j] += matA[i][k] * matB[k][j];
            }
        }
    }
    printf("Matrix multiplication completed.\n");
}

// Função para calcular Fibonacci
long long fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

// Função para verificar números primos
void find_primes() {
    int count = 0;
    for (int i = 2; i < PRIME_LIMIT; i++) {
        int is_prime = 1;
        for (int j = 2; j * j <= i; j++) {
            if (i % j == 0) {
                is_prime = 0;
                break;
            }
        }
        if (is_prime) {
            count++;
        }
    }
    printf("Prime number calculation completed. Total primes: %d\n", count);
}

// Função para rodar as tarefas paralelamente
void* run_benchmark(void* arg) {
    int task = *((int*)arg);

    if (task == 1) {
        matrix_multiply();
    } else if (task == 2) {
        printf("Fibonacci result: %lld\n", fibonacci(FIB_N));
    } else if (task == 3) {
        find_primes();
    }
    return NULL;
}

int main() {
    pthread_t threads[3];
    int tasks[3] = {1, 2, 3};

    // Cria threads para rodar as três tarefas
    for (int i = 0; i < 3; i++) {
        pthread_create(&threads[i], NULL, run_benchmark, &tasks[i]);
    }

    // Espera todas as threads finalizarem
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Benchmark completed.\n");
    return 0;
}

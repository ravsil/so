#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include "mymalloc.h"

#define NUM_ALLOCATIONS 1000000
#define MAX_ALLOCATION_SIZE 2048 // Tamanho máximo de alocação

void benchmark_malloc() {
    void* ptrs[NUM_ALLOCATIONS];

    for (int i = 0; i < NUM_ALLOCATIONS; i++) {
        size_t size = rand() % MAX_ALLOCATION_SIZE + 1;
        ptrs[i] = malloc(size);  // Usando o malloc padrão
    }

    for (int i = 0; i < NUM_ALLOCATIONS; i++) {
        free(ptrs[i]);  // Liberando a memória alocada
    }
}

void benchmark_mymalloc() {
    void* ptrs[NUM_ALLOCATIONS];

    for (int i = 0; i < NUM_ALLOCATIONS; i++) {
        size_t size = rand() % MAX_ALLOCATION_SIZE + 1;
        ptrs[i] = MyMalloc(size);  // Usando o MyMalloc
    }

    for (int i = 0; i < NUM_ALLOCATIONS; i++) {
        MyMallocFree(ptrs[i]);  // Liberando a memória alocada
    }
}

void run_benchmark() {
    clock_t start, end;

    // Benchmark para malloc
    start = clock();
    benchmark_malloc();
    end = clock();
    printf("Benchmark malloc took %.2f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);

    // Benchmark para MyMalloc
    start = clock();
    benchmark_mymalloc();
    end = clock();
    printf("Benchmark MyMalloc took %.2f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
}

int main() {
    run_benchmark();
    return 0;
}

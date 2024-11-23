#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include "mymalloc.h"

#define NUM_ALLOCATIONS 100000
#define MAX_ALLOCATION_SIZE 512 // Tamanho máximo de alocação
#define NUM_THREADS 4  // Número de threads para executar o benchmark

typedef struct {
    int thread_id;
} ThreadData;

void* benchmark_malloc_thread(void* arg) {
    void* ptrs[NUM_ALLOCATIONS];
    ThreadData* data = (ThreadData*)arg;
    printf("Thread %d starting malloc benchmark...\n", data->thread_id);

    for (int i = 0; i < NUM_ALLOCATIONS; i++) {
        size_t size = rand() % MAX_ALLOCATION_SIZE + 1;
        ptrs[i] = malloc(size);  // Usando o malloc padrão
    }

    for (int i = 0; i < NUM_ALLOCATIONS; i++) {
        free(ptrs[i]);  // Liberando a memória alocada
    }

    printf("Thread %d finished malloc benchmark.\n", data->thread_id);
    return NULL;
}

void* benchmark_mymalloc_thread(void* arg) {
    void* ptrs[NUM_ALLOCATIONS];
    ThreadData* data = (ThreadData*)arg;
    printf("Thread %d starting MyMalloc benchmark...\n", data->thread_id);

    for (int i = 0; i < NUM_ALLOCATIONS; i++) {
        size_t size = rand() % MAX_ALLOCATION_SIZE + 1;
        ptrs[i] = MyMalloc(size);  // Usando o MyMalloc
    }

    for (int i = 0; i < NUM_ALLOCATIONS; i++) {
        MyMallocFree(ptrs[i]);  // Liberando a memória alocada
    }

    printf("Thread %d finished MyMalloc benchmark.\n", data->thread_id);
    return NULL;
}

void run_benchmark() {
    clock_t start, end;
    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];
    
    // Benchmark para malloc com múltiplas threads
    start = clock();
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].thread_id = i;
        pthread_create(&threads[i], NULL, benchmark_malloc_thread, (void*)&thread_data[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    end = clock();
    printf("Benchmark malloc with threads took %.2f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);

    // Benchmark para MyMalloc com múltiplas threads
    start = clock();
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].thread_id = i;
        pthread_create(&threads[i], NULL, benchmark_mymalloc_thread, (void*)&thread_data[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    end = clock();
    printf("Benchmark MyMalloc with threads took %.2f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
}

int main() {
    run_benchmark();
    return 0;
}

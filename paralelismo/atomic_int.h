#include <pthread.h>

typedef struct atomic_int
{
    int value;
    pthread_mutex_t lock;
} AtomicInt;

void cria_atomic(AtomicInt *atomic, int value)
{
    atomic->value = value;
    pthread_mutex_init(&atomic->lock, NULL);
}

int atomic_get(AtomicInt *atomic)
{
    pthread_mutex_lock(&atomic->lock);
    int value = atomic->value;
    pthread_mutex_unlock(&atomic->lock);
    return value;
}

void atomic_set(AtomicInt *atomic, int value)
{
    pthread_mutex_lock(&atomic->lock);
    atomic->value = value;
    pthread_mutex_unlock(&atomic->lock);
}

void atomic_increment(AtomicInt *atomic)
{
    pthread_mutex_lock(&atomic->lock);
    atomic->value++;
    pthread_mutex_unlock(&atomic->lock);
}

void atomic_decrement(AtomicInt *atomic)
{
    pthread_mutex_lock(&atomic->lock);
    atomic->value--;
    pthread_mutex_unlock(&atomic->lock);
}

void atomic_sum(AtomicInt *atomic, int value)
{
    pthread_mutex_lock(&atomic->lock);
    atomic->value += value;
    pthread_mutex_unlock(&atomic->lock);
}

void excluir_atomic(AtomicInt *atomic)
{
    pthread_mutex_destroy(&atomic->lock);
}
#pragma once
#include "mutex.h"

typedef struct atomic_int
{
    int value;
    Mutex lock;
} AtomicInt;

void cria_atomic(AtomicInt *atomic, int value)
{
    atomic->value = value;
    cria_mutex(&atomic->lock);
}

int atomic_get(AtomicInt *atomic)
{
    mutex_lock(&atomic->lock);
    int value = atomic->value;
    mutex_unlock(&atomic->lock);
    return value;
}

void atomic_set(AtomicInt *atomic, int value)
{
    mutex_lock(&atomic->lock);
    atomic->value = value;
    mutex_unlock(&atomic->lock);
}

void atomic_increment(AtomicInt *atomic)
{
    mutex_lock(&atomic->lock);
    atomic->value++;
    mutex_unlock(&atomic->lock);
}

void atomic_decrement(AtomicInt *atomic)
{
    mutex_lock(&atomic->lock);
    atomic->value--;
    mutex_unlock(&atomic->lock);
}

void atomic_sum(AtomicInt *atomic, int value)
{
    mutex_lock(&atomic->lock);
    atomic->value += value;
    mutex_unlock(&atomic->lock);
}

void excluir_atomic(AtomicInt *atomic)
{
    excluir_mutex(&atomic->lock);
}
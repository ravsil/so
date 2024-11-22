#pragma once
#include <stdio.h>
#include "mutex.h"
#include "atomic_int.h"

typedef struct cond
{
    int wait_count;
    int signal_pending;
    Mutex internal_mutex;
} Cond;

AtomicInt nThreads;
int first = 1;

void cria_cond(Cond *cond)
{
    cond->wait_count = 0;
    cond->signal_pending = 0;
    cria_mutex(&cond->internal_mutex);
}

void cond_wait(Cond *cond, Mutex *external_mutex)
{
    atomic_decrement(&nThreads);
    if (atomic_get(&nThreads) == 0)
    {
        perror("Todas as threads estão esperando, deadlock detectado");
        exit(1);
    }
    mutex_lock(&cond->internal_mutex);
    cond->wait_count++;
    mutex_unlock(&cond->internal_mutex);

    mutex_unlock(external_mutex);
    while (1)
    {
        if (atomic_get(&nThreads) == 0)
        {
            exit(1);
        }
        mutex_lock(&cond->internal_mutex);
        if (cond->signal_pending && cond->wait_count > 0)
        {
            cond->signal_pending = 0;
            cond->wait_count--;
            mutex_unlock(&cond->internal_mutex);
            break;
        }
        mutex_unlock(&cond->internal_mutex);
    }
    atomic_increment(&nThreads);

    mutex_lock(external_mutex);
}

void cond_signal(Cond *cond)
{
    mutex_lock(&cond->internal_mutex);
    if (cond->wait_count > 0)
    {
        cond->signal_pending = 1;
    }
    mutex_unlock(&cond->internal_mutex);
}

void cond_broadcast(Cond *cond)
{
    mutex_lock(&cond->internal_mutex);
    cond->signal_pending = 1;
    mutex_unlock(&cond->internal_mutex);
}

void excluir_cond(Cond *cond)
{
    excluir_mutex(&cond->internal_mutex);
}

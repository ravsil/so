#pragma once

typedef struct mutex
{
    volatile int locked;
} Mutex;

void cria_mutex(Mutex *mutex)
{
    mutex->locked = 0;
}

void mutex_lock(Mutex *mutex)
{
    while (__sync_lock_test_and_set(&mutex->locked, 1))
        ;
}

void mutex_unlock(Mutex *mutex)
{
    __sync_lock_release(&mutex->locked);
}

void excluir_mutex(Mutex *mutex)
{
    mutex->locked = 0;
}
#pragma once
#include "mutex.h"
#include "cond.h"

typedef struct semaforo
{
    int value;
    Mutex lock;
    Cond cond;
} Semaforo;

void cria_semaforo(Semaforo *semaforo, int value)
{
    semaforo->value = value;
    cria_mutex(&semaforo->lock);
    cria_cond(&semaforo->cond);
}

void up(Semaforo *semaforo)
{
    mutex_lock(&semaforo->lock);
    semaforo->value++;
    cond_signal(&semaforo->cond);
    mutex_unlock(&semaforo->lock);
}

void down(Semaforo *semaforo)
{
    mutex_lock(&semaforo->lock);
    while (semaforo->value == 0)
    {
        cond_wait(&semaforo->cond, &semaforo->lock);
    }
    semaforo->value--;
    mutex_unlock(&semaforo->lock);
}

void excluir_semaforo(Semaforo *semaforo)
{
    excluir_mutex(&semaforo->lock);
    excluir_cond(&semaforo->cond);
}
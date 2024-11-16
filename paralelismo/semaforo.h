#include <pthread.h>

typedef struct semaforo
{
    int value;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} Semaforo;

void cria_semaforo(Semaforo *semaforo, int value)
{
    semaforo->value = value;
    pthread_mutex_init(&semaforo->lock, NULL);
    pthread_cond_init(&semaforo->cond, NULL);
}

void up(Semaforo *semaforo)
{
    pthread_mutex_lock(&semaforo->lock);
    semaforo->value++;
    pthread_cond_signal(&semaforo->cond);
    pthread_mutex_unlock(&semaforo->lock);
}

void down(Semaforo *semaforo)
{
    pthread_mutex_lock(&semaforo->lock);
    while (semaforo->value == 0)
    {
        pthread_cond_wait(&semaforo->cond, &semaforo->lock);
    }
    semaforo->value--;
    pthread_mutex_unlock(&semaforo->lock);
}

void excluir_semaforo(Semaforo *semaforo)
{
    pthread_mutex_destroy(&semaforo->lock);
    pthread_cond_destroy(&semaforo->cond);
}
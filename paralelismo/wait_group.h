#include <pthread.h>

typedef struct wait_group
{
    int count;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} WaitGroup;

void cria_wait_group(WaitGroup *wg)
{
    wg->count = 0;
    pthread_mutex_init(&wg->lock, NULL);
    pthread_cond_init(&wg->cond, NULL);
}

void add(WaitGroup *wg, int n)
{
    pthread_mutex_lock(&wg->lock);
    wg->count += n;
    pthread_mutex_unlock(&wg->lock);
}

void done(WaitGroup *wg)
{
    pthread_mutex_lock(&wg->lock);
    wg->count--;
    pthread_cond_signal(&wg->cond);
    pthread_mutex_unlock(&wg->lock);
}

void wait(WaitGroup *wg)
{
    pthread_mutex_lock(&wg->lock);
    while (wg->count > 0)
    {
        pthread_cond_wait(&wg->cond, &wg->lock);
    }
    pthread_mutex_unlock(&wg->lock);
}

void excluir_wait_group(WaitGroup *wg)
{
    pthread_mutex_destroy(&wg->lock);
    pthread_cond_destroy(&wg->cond);
}

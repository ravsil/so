#pragma once
#include "mutex.h"
#include "cond.h"

typedef struct wait_group
{
    int count;
    Mutex lock;
    Cond cond;
} WaitGroup;

void cria_wait_group(WaitGroup *wg)
{
    wg->count = 0;
    cria_mutex(&wg->lock);
    cria_cond(&wg->cond);
}

void add(WaitGroup *wg, int n)
{
    mutex_lock(&wg->lock);
    wg->count += n;
    mutex_unlock(&wg->lock);
}

void done(WaitGroup *wg)
{
    mutex_lock(&wg->lock);
    wg->count--;
    cond_signal(&wg->cond);
    mutex_unlock(&wg->lock);
}

void wait(WaitGroup *wg)
{
    mutex_lock(&wg->lock);
    while (wg->count > 0)
    {
        cond_wait(&wg->cond, &wg->lock);
    }
    mutex_unlock(&wg->lock);
}

void excluir_wait_group(WaitGroup *wg)
{
    excluir_mutex(&wg->lock);
    excluir_cond(&wg->cond);
}

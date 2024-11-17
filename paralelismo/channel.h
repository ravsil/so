#pragma once
#include <stdlib.h>

#include "mutex.h"
#include "cond.h"

typedef struct channel
{
    int *buffer;
    int size;
    int start;
    int end;
    int open;
    Mutex lock;
    Cond cond;
} Channel;

void cria_channel(Channel *channel, int size)
{
    channel->buffer = (int *)malloc(size * sizeof(int));
    channel->size = size + 1;
    channel->start = 0;
    channel->end = 0;
    channel->open = 1;
    cria_mutex(&channel->lock);
    cria_cond(&channel->cond);
}

void send(Channel *channel, int value)
{
    if (!channel->open)
    {
        return;
    }

    mutex_lock(&channel->lock);
    while ((channel->end + 1) % channel->size == channel->start)
    {
        cond_wait(&channel->cond, &channel->lock);
    }
    channel->buffer[channel->end] = value;
    channel->end = (channel->end + 1) % channel->size;
    cond_signal(&channel->cond);
    mutex_unlock(&channel->lock);
}

void receive(Channel *channel, int *value)
{
    mutex_lock(&channel->lock);
    while (channel->start == channel->end && channel->open)
    {
        cond_wait(&channel->cond, &channel->lock);
    }
    *value = (channel->open) ? channel->buffer[channel->start] : 0;
    channel->start = (channel->start + 1) % channel->size;
    cond_signal(&channel->cond);
    mutex_unlock(&channel->lock);
}

void fechar_channel(Channel *channel)
{
    mutex_lock(&channel->lock);
    channel->open = 0;
    cond_broadcast(&channel->cond);
    mutex_unlock(&channel->lock);
}

void excluir_channel(Channel *channel)
{
    free(channel->buffer);
    excluir_mutex(&channel->lock);
    excluir_cond(&channel->cond);
}
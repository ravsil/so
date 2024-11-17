#pragma once
#include <stdlib.h>

#include "mutex.h"
#include "cond.h"

typedef struct complex_channel
{
    void **buffer;
    int size;
    int start;
    int end;
    int open;
    Mutex lock;
    Cond cond;
} ComplexChannel;

void cria_complex_channel(ComplexChannel *channel, int size)
{
    channel->buffer = (void **)malloc(size * sizeof(void *));
    channel->size = size + 1;
    channel->start = 0;
    channel->end = 0;
    channel->open = 1;
    cria_mutex(&channel->lock);
    cria_cond(&channel->cond);
}

void send_complex(ComplexChannel *channel, void *value)
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

void receive_complex(ComplexChannel *channel, void **value)
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

void fechar_complex_channel(ComplexChannel *channel)
{
    mutex_lock(&channel->lock);
    channel->open = 0;
    cond_broadcast(&channel->cond);
    mutex_unlock(&channel->lock);
}

void excluir_complex_channel(ComplexChannel *channel)
{
    free(channel->buffer);
    excluir_mutex(&channel->lock);
    excluir_cond(&channel->cond);
}
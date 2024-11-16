#include <pthread.h>
#include <stdlib.h>

typedef struct channel
{
    int *buffer;
    int size;
    int start;
    int end;
    int open;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} Channel;

void cria_channel(Channel *channel, int size)
{
    channel->buffer = (int *)malloc(size * sizeof(int));
    channel->size = size + 1;
    channel->start = 0;
    channel->end = 0;
    channel->open = 1;
    pthread_mutex_init(&channel->lock, NULL);
    pthread_cond_init(&channel->cond, NULL);
}

void send(Channel *channel, int value)
{
    if (!channel->open)
    {
        return;
    }

    pthread_mutex_lock(&channel->lock);
    while ((channel->end + 1) % channel->size == channel->start)
    {
        pthread_cond_wait(&channel->cond, &channel->lock);
    }
    channel->buffer[channel->end] = value;
    channel->end = (channel->end + 1) % channel->size;
    pthread_cond_signal(&channel->cond);
    pthread_mutex_unlock(&channel->lock);
}

void receive(Channel *channel, int *value)
{
    pthread_mutex_lock(&channel->lock);
    while (channel->start == channel->end && channel->open)
    {
        pthread_cond_wait(&channel->cond, &channel->lock);
    }
    *value = (channel->open) ? channel->buffer[channel->start] : 0;
    channel->start = (channel->start + 1) % channel->size;
    pthread_cond_signal(&channel->cond);
    pthread_mutex_unlock(&channel->lock);
}

void fechar_channel(Channel *channel)
{
    pthread_mutex_lock(&channel->lock);
    channel->open = 0;
    pthread_cond_broadcast(&channel->cond);
    pthread_mutex_unlock(&channel->lock);
}

void excluir_channel(Channel *channel)
{
    free(channel->buffer);
    pthread_mutex_destroy(&channel->lock);
    pthread_cond_destroy(&channel->cond);
}
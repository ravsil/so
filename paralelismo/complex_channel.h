#include <pthread.h>
#include <stdlib.h>

typedef struct complex_channel
{
    void **buffer;
    int size;
    int start;
    int end;
    int open;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} ComplexChannel;

void cria_complex_channel(ComplexChannel *channel, int size)
{
    channel->buffer = (void **)malloc(size * sizeof(void *));
    channel->size = size + 1;
    channel->start = 0;
    channel->end = 0;
    channel->open = 1;
    pthread_mutex_init(&channel->lock, NULL);
    pthread_cond_init(&channel->cond, NULL);
}

void send_complex(ComplexChannel *channel, void *value)
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

void receive_complex(ComplexChannel *channel, void **value)
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

void fechar_complex_channel(ComplexChannel *channel)
{
    pthread_mutex_lock(&channel->lock);
    channel->open = 0;
    pthread_cond_broadcast(&channel->cond);
    pthread_mutex_unlock(&channel->lock);
}

void excluir_complex_channel(ComplexChannel *channel)
{
    free(channel->buffer);
    pthread_mutex_destroy(&channel->lock);
    pthread_cond_destroy(&channel->cond);
}
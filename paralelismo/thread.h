#pragma once
#include <sched.h>
#include <stdlib.h>
#include <stdio.h>
#include "wait_group.h"
#include "cond.h"

#define THREAD_STACK_SIZE 1024 * 1024
#define CLONE_FS 0x00000200
#define CLONE_FILES 0x00000400
#define CLONE_SIGHAND 0x00000800
#define CLONE_VM 0x00000100

int clone(int (*fn)(void *), void *child_stack, int flags, void *arg);

struct wrapper_args
{
    void (*func)(void *);
    void *arg;
    WaitGroup *wg;
};

static int thread_wrapper(void *wrapper_arg)
{
    struct wrapper_args *args = (struct wrapper_args *)wrapper_arg;
    args->func(args->arg);

    if (args->wg)
    {
        done(args->wg);
    }
    atomic_decrement(&nThreads);

    free(args);
    return 0;
}

int cria_thread_generico(void (*func)(void *), void *arg, WaitGroup *wg)
{
    if (first)
    {
        first = 0;
        cria_atomic(&nThreads, 0);
        atomic_increment(&nThreads); // conta a thread principal
    }
    atomic_increment(&nThreads);

    void *stack = malloc(THREAD_STACK_SIZE);
    if (!stack)
    {
        perror("Falha na alocacao da pilha");
        return -1;
    }

    struct wrapper_args *args = malloc(sizeof(struct wrapper_args));
    if (!args)
    {
        perror("Falha na alocacao dos argumentos");
        free(stack);
        return -1;
    }
    args->func = func;
    args->arg = arg;
    args->wg = wg;

    if (wg)
    {
        add(wg, 1);
    }

    int result = clone(thread_wrapper, stack + THREAD_STACK_SIZE, CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_VM, args);
    if (result == -1)
    {
        perror("Falha na criacao da thread");
        free(stack);
        free(args);
    }

    return result;
}

#define cria_thread(func, arg, wg) cria_thread_generico((void (*)(void *))(func), (void *)(arg), wg)
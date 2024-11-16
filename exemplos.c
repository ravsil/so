#include <stdio.h>
#include "paralelismo/paralelismo.h"
#include <pthread.h>

void total(AtomicInt *tally)
{
    for (int i = 0; i < 50; i++)
    {
        atomic_increment(tally);
    }
}

void sender(Channel *ch)
{
    for (int i = 0; i < 10; i++)
    {
        send(ch, i);
    }
}

void complex_sender(ComplexChannel *cch)
{
    send_complex(cch, "Hello, World!");
}

typedef struct args
{
    Semaforo sem;
    WaitGroup wg;
    int val;
} Args;

void somador(Args *args)
{
    for (int i = 0; i < 50; i++)
    {
        down(&args->sem); // decrementa o semáforo
        args->val += 1;   // região crítica
        up(&args->sem);   // incrementa o semáforo
    }
    done(&args->wg); // sinaliza que a thread terminou
}

int main()
{
    // inteiro atômico, permite apenas que uma thread acessar o valor por vez
    AtomicInt tally;
    cria_atomic(&tally, 0);

    pthread_t t1, t2;
    // a função total incrementa o valor de tally 50 vezes
    pthread_create(&t1, NULL, (void *)total, &tally);
    pthread_create(&t2, NULL, (void *)total, &tally);
    pthread_join(t1, NULL); // espera a 1 thread terminar
    pthread_join(t2, NULL); // espera a 2 thread terminar
    // o valor impresso sempre será o mesmo devido à natureza do tipo atômico
    printf("Total: %d\n", atomic_get(&tally));
    excluir_atomic(&tally);

    // channels permitem a troca de mensagens entre threads
    Channel ch;
    cria_channel(&ch, 10); // cria um channel com espaço para 10 valores
    pthread_t t3;
    // a função sender envia 10 valores para o canal
    pthread_create(&t3, NULL, (void *)sender, &ch);
    for (int i = 0; i < 10; i++)
    {
        int value;
        // a função receive bloqueia a execução até que um valor seja recebido
        receive(&ch, &value);
        printf("Valor recebido: %d\n", value);
    }
    excluir_channel(&ch);

    // complex channels permitem a troca de mensagens de qualquer tipo
    ComplexChannel cch;
    cria_complex_channel(&cch, 1); // cria um complex channel com espaço para 1 valor
    pthread_t t4;
    // a função complex_sender envia uma string para o canal
    pthread_create(&t4, NULL, (void *)complex_sender, &cch);
    char *value;
    // bloqueia a execução até que um valor seja recebido
    receive_complex(&cch, (void **)&value);
    printf("Valor recebido: %s\n", value);
    excluir_complex_channel(&cch);

    // Como não é possível enviar múltiplos argumentos para uma thread, é necessário criar uma struct
    Args args;
    args.val = 0;
    // cria um semáforo com valor inicial 1
    cria_semaforo(&args.sem, 1);
    // cria um wait group
    cria_wait_group(&args.wg);
    // adiciona 2 threads à fila de espera
    add(&args.wg, 2);
    pthread_t t5, t6;
    // a função somador incrementa o valor de args->val 50 vezes
    pthread_create(&t5, NULL, (void *)somador, &args);
    pthread_create(&t6, NULL, (void *)somador, &args);
    wait(&args.wg); // espera as threads terminarem
    printf("Valor: %d\n", args.val);
    excluir_semaforo(&args.sem);
    excluir_wait_group(&args.wg);

    return 0;
}
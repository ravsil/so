#include <stdio.h>
#include "paralelismo/paralelismo.h"

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
}

int main()
{
    // inteiro atômico, permite apenas que uma thread acessar o valor por vez
    AtomicInt tally;
    WaitGroup wg;
    cria_atomic(&tally, 0);
    cria_wait_group(&wg);

    // a função total incrementa o valor de tally 50 vezes
    cria_thread(total, &tally, &wg);
    cria_thread(total, &tally, &wg);
    wait(&wg); // espera as threads terminarem

    // o valor impresso sempre será o mesmo devido à natureza do tipo atômico
    printf("Total: %d\n", atomic_get(&tally));
    excluir_atomic(&tally);
    excluir_wait_group(&wg);

    // channels permitem a troca de mensagens entre threads
    Channel ch;
    cria_channel(&ch, 10); // cria um channel com espaço para 10 valores

    // a função sender envia 10 valores para o canal
    cria_thread(sender, &ch, NULL);
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

    // a função complex_sender envia uma string para o canal
    cria_thread(complex_sender, &cch, NULL);
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

    cria_wait_group(&wg);
    // a função somador incrementa o valor de args->val 50 vezes
    cria_thread(somador, &args, &wg);
    cria_thread(somador, &args, &wg);
    cria_thread(somador, &args, &wg);
    wait(&wg); // espera as threads terminarem
    printf("Valor: %d\n", args.val);
    excluir_semaforo(&args.sem);
    excluir_wait_group(&wg);

    return 0;
}
/* 
Ferramentas como prstat (Solaris) e plockstat ajudam a identificar contenção em malloc() e free(), fornecendo métricas como:
 - Porcentagem de tempo gasto esperando bloqueios.
 - Contagem de giros (spinlocks) e tempo médio gasto bloqueado.
 
Soluções:
 - Freelists Segregadas: Talvez mt complexidade atoa
 - Coalescing: Coalescing é útil porque resolve um problema importante chamado fragmentação externa, que ocorre quando há muitos blocos pequenos e não contíguos de memória livre, que não podem ser usados para alocações maiores, mesmo que no total haja memória suficiente disponível.
 - Splitting: Splitting é útil para lidar com a fragmentação interna, que ocorre quando um bloco de memória é alocado e não é completamente utilizado. A divisão de blocos permite que o espaço restante seja reaproveitado, melhorando a utilização da memória.



TODO: Melhorar a implementação para multiprocessamento
Soluções:

Evitar Contenção de Locks:
Em ambientes multithread, um dos maiores desafios é a contenção de locks. Quando múltiplos threads acessam a memória simultaneamente, o uso de locks para garantir a integridade dos dados pode levar a desempenho reduzido devido ao bloqueio de recursos.
Para resolver isso, podemos usar técnicas como freelists segregadas e locks finos, ou até mesmo uma abordagem sem bloqueio usando uma estrutura otimizada para o acesso concorrente, como atomic operations.

Segregação de Freelists:
Usar diferentes listas de blocos livres para diferentes tamanhos de blocos pode ajudar a reduzir a contenção. Ao dividir a lista de blocos livres em várias listas para diferentes faixas de tamanhos de blocos, minimizamos o impacto de blocos livres concorrentes.

Locks Finitos (Fine-Grained Locks):
Ao invés de ter um único lock global para toda a gestão de memória, podemos utilizar locks finos (por exemplo, um lock por freelist), o que diminui a chance de múltiplos threads ficarem bloqueados esperando o acesso a um único lock.

Atomicidade e Operações Sem Bloqueio:
Para operações críticas, podemos usar atomic operations fornecidas pela arquitetura de hardware, como operações atômicas de incremento e comparação, para garantir que múltiplos threads possam operar sobre a memória sem bloqueios pesados.

 */




#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>  // Para mmap, que gerencia o heap diretamente
#include <pthread.h>    // Para trabalhar com múltiplos threads
#include <stdatomic.h>  // Para operações atômicas, se necessário

#define BLOCK_SIZE sizeof(Block)  // Tamanho da estrutura de controle de cada bloco de memória

// Estrutura que representa um bloco de memória
typedef struct Block {
    size_t size;          // Tamanho do bloco de memória gerenciado
    struct Block* next;   // Ponteiro para o próximo bloco na lista
    int free;             // Status do bloco: 1 (livre) ou 0 (ocupado)
} Block;

// Ponteiro para o início do heap gerenciado
void* heap_start = NULL;  // Armazena a base do espaço de memória alocado
Block* free_lists[16] = { NULL };  // Lista de freelists segmentada por tamanho de bloco
pthread_mutex_t freelist_locks[16];  // Locks para as listas de freelists

/**
 * Função para calcular o índice da freelist com base no tamanho do bloco.
 * Divide o tamanho do bloco para mapear a um índice de freelist específico.
 *
 * @param size O tamanho do bloco a ser alocado
 * @return O índice da freelist que corresponde ao tamanho do bloco
 */
int get_freelist_index(size_t size) {
    // O índice é determinado por uma divisão do tamanho por 8 para reduzir a segmentação
    size_t bin_size = size >> 3;  // Reduz a faixa de tamanhos de blocos
    return (bin_size < 16) ? bin_size : 15;  // Limita ao máximo de 16 freelists
}

/**
 * Função para inserir um bloco na lista de blocos livres.
 * A inserção é feita de forma ordenada para facilitar a coalescência futura.
 *
 * @param block Bloco a ser inserido na lista de blocos livres
 */
void insert_free_block(Block* block) {
    int index = get_freelist_index(block->size);  // Obtem o índice da freelist para o tamanho do bloco

    pthread_mutex_lock(&freelist_locks[index]);  // Bloqueia o acesso à freelist correspondente

    // Inserção do bloco na freelist de maneira ordenada
    block->next = free_lists[index];
    free_lists[index] = block;

    pthread_mutex_unlock(&freelist_locks[index]);  // Libera o lock da freelist
}

/**
 * Função para coalescer blocos adjacentes que estão livres.
 * A coalescência junta blocos de memória contíguos, aumentando o espaço livre contínuo.
 */
void coalesce_free_blocks() {
    for (int i = 0; i < 16; i++) {
        pthread_mutex_lock(&freelist_locks[i]);  // Bloqueia a freelist para coalescência

        Block* current = free_lists[i];
        while (current && current->next) {
            // Verifica se o bloco atual e o próximo estão livres
            if (current->free && current->next->free) {
                // Junta os blocos adjacentes
                current->size += current->next->size + BLOCK_SIZE;
                current->next = current->next->next;
            } else {
                current = current->next;  // Move para o próximo bloco
            }
        }

        pthread_mutex_unlock(&freelist_locks[i]);  // Libera o lock da freelist
    }
}

/**
 * Função que inicializa o gerenciador de memória.
 * A memória do heap é alocada com mmap e a primeira freelist é configurada.
 *
 * @param size Tamanho total do heap a ser gerenciado
 * @return Retorna 1 em caso de sucesso, 0 em caso de falha
 */
int MyMallocInit(int size) {
    // Solicita um bloco contínuo de memória ao sistema operacional com mmap
    heap_start = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    
    if (heap_start == MAP_FAILED) {  // Verifica se a alocação falhou
        return 0;  // Falha na alocação
    }

    // Inicializa a lista de blocos livres (freelist) com o primeiro bloco
    Block* initial_block = (Block*)heap_start;
    initial_block->size = size - BLOCK_SIZE;  // O tamanho disponível é o total menos o tamanho do cabeçalho
    initial_block->next = NULL;  // Não há outros blocos
    initial_block->free = 1;     // O bloco começa como livre

    int index = get_freelist_index(initial_block->size);  // Calcula a freelist para o bloco
    free_lists[index] = initial_block;  // Insere o primeiro bloco na freelist apropriada

    // Inicializa os locks para todas as freelists
    for (int i = 0; i < 16; i++) {
        pthread_mutex_init(&freelist_locks[i], NULL);
    }

    return 1;  // Sucesso
}

/**
 * Função que aloca memória de tamanho especificado.
 * Procura um bloco livre adequado e o divide se necessário.
 *
 * @param size O tamanho da memória a ser alocada
 * @return Retorna um ponteiro para a memória alocada ou NULL se não houver espaço suficiente
 */
void* MyMalloc(int size) {
    int index = get_freelist_index(size);  // Obtem o índice da freelist para o tamanho solicitado
    Block* current = free_lists[index];    // Começa na freelist correspondente

    pthread_mutex_lock(&freelist_locks[index]);  // Bloqueia a freelist para operação segura

    while (current) {
        // Verifica se o bloco é livre e grande o suficiente
        if (current->free && current->size >= size) {
            current->free = 0;  // Marca o bloco como ocupado

            // Se o bloco for grande o suficiente, divide-o em dois
            if (current->size > size + BLOCK_SIZE) {
                // Cria um novo bloco com o espaço restante
                Block* new_block = (Block*)((char*)current + BLOCK_SIZE + size);
                new_block->size = current->size - size - BLOCK_SIZE;
                new_block->free = 1;  // O novo bloco é livre
                new_block->next = current->next;

                // Atualiza o bloco atual
                current->size = size;
                current->next = new_block;
            }

            pthread_mutex_unlock(&freelist_locks[index]);  // Libera o lock da freelist
            return (void*)((char*)current + BLOCK_SIZE);  // Retorna o ponteiro para a área utilizável
        }

        // Move para o próximo bloco
        current = current->next;
    }

    pthread_mutex_unlock(&freelist_locks[index]);  // Libera o lock da freelist
    return NULL;  // Se não encontrou um bloco adequado
}

/**
 * Função que libera um bloco de memória previamente alocado.
 * Insere o bloco de volta na freelist e tenta coalescer blocos adjacentes.
 *
 * @param ptr Ponteiro para o início do bloco alocado
 * @return Retorna 1 em caso de sucesso, 0 se o ponteiro for inválido
 */
int MyMallocFree(void* ptr) {
    if (!ptr) return 0;  // Verifica se o ponteiro é nulo

    Block* block = (Block*)((char*)ptr - sizeof(Block));  // Obtém o cabeçalho do bloco
    block->free = 1;  // Marca o bloco como livre

    int index = get_freelist_index(block->size);  // Calcula a freelist para o bloco
    insert_free_block(block);  // Insere o bloco na freelist

    coalesce_free_blocks();  // Tenta coalescer blocos adjacentes livres

    return 1;  // Sucesso
}
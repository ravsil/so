/* 
Ferramentas como prstat (Solaris) e plockstat ajudam a identificar contenção em malloc() e free(), fornecendo métricas como:
 - Porcentagem de tempo gasto esperando bloqueios.
 - Contagem de giros (spinlocks) e tempo médio gasto bloqueado.
 
Soluções:
 - Freelists Segregadas: Talvez mt complexidade atoa
 - Coalescing: Coalescing é útil porque resolve um problema importante chamado fragmentação externa, que ocorre quando há muitos blocos pequenos e não contíguos de memória livre, que não podem ser usados para alocações maiores, mesmo que no total haja memória suficiente disponível.
 - Splitting: Splitting é útil para lidar com a fragmentação interna, que ocorre quando um bloco de memória é alocado e não é completamente utilizado. A divisão de blocos permite que o espaço restante seja reaproveitado, melhorando a utilização da memória.



Melhorar a implementação para multiprocessamento
Soluções:

Evitar Contenção de Locks:
Em ambientes multithread, um dos maiores desafios é a contenção de locks. Quando múltiplos threads acessam a memória simultaneamente, o uso de locks para garantir a integridade dos dados pode levar a desempenho reduzido devido ao bloqueio de recursos.
Para resolver isso, podemos usar técnicas como freelists segregadas e locks finos, ou até mesmo uma abordagem sem bloqueio usando uma estrutura otimizada para o acesso concorrente, como atomic operations.

Segregação de Freelists:
Usar diferentes listas de blocos livres para diferentes tamanhos de blocos pode ajudar a reduzir a contenção. Ao dividir a lista de blocos livres em várias listas para diferentes faixas de tamanhos de blocos, minimizamos o impacto de blocos livres concorrentes.

Locks Finitos (Fine-Grained Locks):
Ao invés de ter um único lock global para toda a gestão de memória, podemos utilizar locks finos (por exemplo, um lock por freelist), o que diminui a chance de múltiplos threads ficarem bloqueados esperando o acesso a um único lock.

Soluções V2:
Usei TLS para armazenar a arena de cada thread, evitando a necessidade de locks globais, no caso vai ter só se a arena estiver cheia, assim ele aloca na heap global, tendo um lock pra isso também. Isso garante que as alocações nunca falhem, desde que haja memória disponível no sistema e cada thread inicializa sua arena apenas quando faz a primeira alocação. Isso reduz a contenção de locks e melhora o desempenho em ambientes multithread.

 */



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>
#include <stdatomic.h>

#define BLOCK_SIZE sizeof(Block)  // Tamanho do cabeçalho de um bloco
#define HEAP_CHUNK_SIZE 4096      // Tamanho de alocação em blocos do heap global

// Estrutura que representa um bloco de memória
typedef struct Block {
    size_t size;           // Tamanho do bloco (não inclui o cabeçalho)
    struct Block* next;    // Ponteiro para o próximo bloco na freelist
    struct Block* prev;    // Ponteiro para o bloco anterior (necessário para coalescência)
    int free;              // Status do bloco (1: livre, 0: ocupado)
} Block;

// Estrutura que representa uma arena
typedef struct Arena {
    Block* free_lists[16];             // Freelists segmentadas por tamanho
    pthread_mutex_t freelist_locks[16]; // Locks para cada freelist
} Arena;

// Variável thread-local para armazenar a arena de cada thread
__thread Arena* thread_arena = NULL;

// Heap global e seu lock para fallback
void* global_heap_start = NULL;
void* global_heap_end = NULL;
pthread_mutex_t global_heap_lock = PTHREAD_MUTEX_INITIALIZER;

/**
 * Calcula o índice da freelist com base no tamanho do bloco.
 * O tamanho é arredondado para múltiplos de 8 bytes, e o índice é calculado com base nessa faixa.
 * 
 * @param size Tamanho do bloco solicitado.
 * @return O índice da freelist apropriada.
 */
int get_freelist_index(size_t size) {
    size_t bin_size = size >> 3;  // Divide o tamanho por 8 para determinar a faixa
    return (bin_size < 16) ? bin_size : 15;  // Limita ao índice máximo
}

/**
 * Inicializa uma nova arena para o thread.
 * Cada thread possui sua própria arena, composta de freelists segmentadas por tamanhos.
 * 
 * @return Ponteiro para a arena inicializada ou NULL se falhar.
 */
Arena* initialize_arena() {
    Arena* arena = (Arena*)malloc(sizeof(Arena));
    if (!arena) return NULL;

    // Inicializa freelists e locks
    for (int i = 0; i < 16; i++) {
        arena->free_lists[i] = NULL;
        pthread_mutex_init(&arena->freelist_locks[i], NULL);
    }
    return arena;
}

/**
 * Garante que a arena do thread atual foi inicializada.
 * Se ainda não houver uma arena associada ao thread, ela será criada.
 */
void ensure_thread_arena() {
    if (!thread_arena) {
        thread_arena = initialize_arena();
    }
}

/**
 * Inicializa o heap global.
 * O heap global é usado como fallback quando uma arena local não consegue atender a uma solicitação.
 * 
 * @param size Tamanho total do heap global.
 * @return 1 em caso de sucesso, 0 em caso de falha.
 */
int MyMallocInit(int size) {
    pthread_mutex_lock(&global_heap_lock);

    if (!global_heap_start) {
        global_heap_start = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (global_heap_start == MAP_FAILED) {
            pthread_mutex_unlock(&global_heap_lock);
            return 0;
        }
        global_heap_end = (void*)((char*)global_heap_start + size);
    }

    pthread_mutex_unlock(&global_heap_lock);
    return 1;
}

/**
 * Aloca memória do heap global como fallback.
 * Essa função é chamada quando nenhuma arena local consegue atender à solicitação.
 * 
 * @param size Tamanho da memória solicitada.
 * @return Ponteiro para a memória alocada ou NULL se falhar.
 */
void* allocate_from_global_heap(size_t size) {
    pthread_mutex_lock(&global_heap_lock);

    // Verifica se há espaço suficiente no heap global
    if ((char*)global_heap_start + size + BLOCK_SIZE > (char*)global_heap_end) {
        pthread_mutex_unlock(&global_heap_lock);
        return NULL; // Sem memória disponível
    }

    Block* block = (Block*)global_heap_start;
    block->size = size;
    block->free = 0;
    block->next = NULL;
    block->prev = NULL;

    global_heap_start = (void*)((char*)global_heap_start + size + BLOCK_SIZE);

    pthread_mutex_unlock(&global_heap_lock);
    return (void*)((char*)block + BLOCK_SIZE);
}

/**
 * Divide um bloco maior em dois, se possível.
 * A divisão ocorre apenas se o bloco original for significativamente maior que o solicitado.
 * 
 * @param block Ponteiro para o bloco a ser dividido.
 * @param size Tamanho solicitado para o novo bloco.
 */
void split_block(Block* block, size_t size) {
    if (block->size > size + BLOCK_SIZE) {
        Block* new_block = (Block*)((char*)block + BLOCK_SIZE + size);
        new_block->size = block->size - size - BLOCK_SIZE;
        new_block->free = 1;
        new_block->next = block->next;
        new_block->prev = block;

        if (new_block->next) {
            new_block->next->prev = new_block;
        }

        block->size = size;
        block->next = new_block;
    }
}

/**
 * Aloca memória na arena local do thread.
 * Se a arena local não puder atender à solicitação, tenta o heap global como fallback.
 * 
 * @param size Tamanho da memória solicitada.
 * @return Ponteiro para a memória alocada ou NULL se não houver espaço suficiente.
 */
void* MyMalloc(int size) {
    ensure_thread_arena(); // Garante que a arena do thread foi inicializada
    int index = get_freelist_index(size);
    Arena* arena = thread_arena;

    pthread_mutex_lock(&arena->freelist_locks[index]);

    Block* current = arena->free_lists[index];
    while (current) {
        if (current->free && current->size >= size) {
            current->free = 0;
            split_block(current, size); // Divide o bloco, se necessário
            pthread_mutex_unlock(&arena->freelist_locks[index]);
            return (void*)((char*)current + BLOCK_SIZE);
        }
        current = current->next;
    }

    pthread_mutex_unlock(&arena->freelist_locks[index]);

    // Tenta alocar no heap global como fallback
    return allocate_from_global_heap(size);
}

/**
 * Libera um bloco de memória previamente alocado.
 * Blocos liberados são reinseridos na freelist correspondente na arena local.
 * 
 * @param ptr Ponteiro para a memória a ser liberada.
 * @return 1 em caso de sucesso, 0 se o ponteiro for inválido.
 */
int MyMallocFree(void* ptr) {
    if (!ptr) return 0;

    Block* block = (Block*)((char*)ptr - BLOCK_SIZE);
    block->free = 1;

    ensure_thread_arena();
    int index = get_freelist_index(block->size);

    pthread_mutex_lock(&thread_arena->freelist_locks[index]);
    block->next = thread_arena->free_lists[index];
    if (thread_arena->free_lists[index]) {
        thread_arena->free_lists[index]->prev = block;
    }
    thread_arena->free_lists[index] = block;
    pthread_mutex_unlock(&thread_arena->freelist_locks[index]);

    return 1;
}


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>  // Para mmap, que gerencia o heap diretamente

#define BLOCK_SIZE sizeof(Block)  // Tamanho da estrutura de controle de cada bloco

// Estrutura para gerenciar blocos de memória
typedef struct Block {
    size_t size;          // Tamanho do bloco de memória gerenciado
    struct Block* next;   // Ponteiro para o próximo bloco na lista
    int free;             // Status do bloco: 1 (livre) ou 0 (ocupado)
} Block;

// Ponteiro para o início do heap gerenciado
void* heap_start = NULL;  // Armazena a base do espaço de memória alocado
Block* free_list = NULL;  // Ponteiro para a lista de blocos livres

/**
 * Inicializa o gerenciador de memória.
 *
 * @param size Tamanho total do heap a ser gerenciado.
 * @return Retorna 1 em caso de sucesso e 0 em caso de falha.
 */
int MyMallocInit(int size) {
    // Solicita um bloco contínuo de memória ao SO usando mmap
    heap_start = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    // Verifica se mmap conseguiu alocar a memória
    if (heap_start == MAP_FAILED) {
        return 0;  // Falha na alocação
    }

    // Inicializa o primeiro bloco no espaço alocado
    free_list = (Block*)heap_start;  // O início do heap é também o início do primeiro bloco
    free_list->size = size - BLOCK_SIZE;  // O tamanho disponível é o total menos o tamanho do cabeçalho
    free_list->next = NULL;  // Ainda não há outros blocos
    free_list->free = 1;     // O bloco começa como livre

    return 1;  // Sucesso
}

/**
 * Aloca memória de tamanho especificado.
 *
 * @param size Tamanho da memória a ser alocada.
 * @return Um ponteiro para a memória alocada, ou NULL se não houver espaço suficiente.
 */
void* MyMalloc(int size) {
    Block* current = free_list;  // Começa no primeiro bloco livre

    // Percorre a lista de blocos procurando um bloco adequado
    while (current != NULL) {
        // Verifica se o bloco é livre e grande o suficiente
        if (current->free && current->size >= size) {
            current->free = 0;  // Marca o bloco como ocupado

            // Verifica se o bloco pode ser dividido
            if (current->size > size + BLOCK_SIZE) {
                // Cria um novo bloco com o espaço restante
                Block* new_block = (Block*)((char*)current + BLOCK_SIZE + size);
                new_block->size = current->size - size - BLOCK_SIZE;
                new_block->free = 1;  // Novo bloco é livre
                new_block->next = current->next;

                // Atualiza o bloco atual
                current->size = size;
                current->next = new_block;
            }

            // Retorna o ponteiro para a área utilizável (logo após o cabeçalho)
            return (void*)((char*)current + BLOCK_SIZE);
        }

        // Passa para o próximo bloco
        current = current->next;
    }

    // Não encontrou um bloco adequado
    return NULL;
}

/**
 * Libera a memória previamente alocada.
 *
 * @param ptr Ponteiro para o início do bloco alocado.
 * @return Retorna 1 em caso de sucesso e 0 se o ponteiro for inválido.
 */
int MyMallocFree(void* ptr) {
    // Verifica se o ponteiro é nulo
    if (!ptr) return 0;

    // Obtém o cabeçalho do bloco associado ao ponteiro
    Block* block = (Block*)((char*)ptr - BLOCK_SIZE);

    // Marca o bloco como livre
    block->free = 1;

    // TODO: Implementar coalescência (união de blocos livres adjacentes)

    return 1;
}

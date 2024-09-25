#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>

#define LIMIT 50  // Limite de processos a serem monitorados

void get_process_info(pid_t pid, int count) {
    if (count >= LIMIT) {
        return;
        }
    char filename[50];
    snprintf(filename, sizeof(filename), "/proc/%d/stat", pid);

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        return;  // Pula processos que não estão mais ativos
    }

    // Variáveis para armazenar os campos
    int ppid, pgrp, session, tty_nr, tpgid;
    unsigned long minflt, cminflt, majflt, cmajflt, utime, stime;
    unsigned int flags;
    char state, comm[256];

    // Ler os campos do arquivo /proc/[pid]/stat
    fscanf(file, "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu",
           &pid, comm, &state, &ppid, &pgrp, &session, &tty_nr, &tpgid,
           &flags, &minflt, &cminflt, &majflt, &cmajflt, &utime, &stime);
    fclose(file);

    // Imprimir todas as informações capturadas de forma formatada
    printf("%-6d | %-40s | %-1c | %-7d | %-5d | %-6d | %-5d | %-12d | %-8u | %-10lu | %-14lu | %-12lu | %-16lu | %-12lu | %lu\n", 
           pid, comm, state, ppid, pgrp, session, tty_nr, tpgid, flags, minflt, cminflt, majflt, cmajflt, utime, stime);
}

void monitor_all_processes() {
    DIR *dir;
    struct dirent *entry;

    // Abrir o diretório /proc
    dir = opendir("/proc");
    if (dir == NULL) {
        perror("Erro ao abrir /proc");
        return;
    }

    // Ler cada diretório em /proc
    int count = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (isdigit(entry->d_name[0])) {
            pid_t pid = atoi(entry->d_name);  // Convertendo o nome do diretório em PID
            get_process_info(pid, count++);  // Monitorar o processo
        }
    }

    closedir(dir);
}

int main() {
    printf("Monitorando todos os processos...\n");

    // Loop de monitoramento
    for (int i = 0; i != -1; i++) {
        system("clear");
        printf("N Proc | Nome                                     | E | PID Pai | Grupo | Sessao |  TTY  | ID Grupo TTY |   Flags  | Faltas Pag | Faltas Pag (C) | Faltas Pag B | Faltas Pag B (C) | Ticks (User) | Ticks (Kernel)\n");
        printf("------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
        monitor_all_processes();
        printf("\n--- Fim da iteração %d ---\n\n", i + 1);
        sleep(10);  // Esperar 1 segundo entre as leituras
    }

    return 0;
}

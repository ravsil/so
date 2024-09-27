#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define PROC_DIR "/proc"

// Function to get the process name from /proc/[pid]/comm
void get_process_name(pid_t pid, char *name) {
    char path[40];
    snprintf(path, sizeof(path), "/proc/%d/comm", pid);
    
    FILE *f = fopen(path, "r");
    if (f) {
        fgets(name, 256, f);
        name[strcspn(name, "\n")] = 0;  // Remove newline character
        fclose(f);
    } else {
        strcpy(name, "Unknown");
    }
}

// Function to get process state from /proc/[pid]/stat
char get_process_state(pid_t pid) {
    char path[40];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);

    FILE *f = fopen(path, "r");
    if (f) {
        int pid_dummy;
        char comm[256], state;
        fscanf(f, "%d %s %c", &pid_dummy, comm, &state);
        fclose(f);
        return state;
    } else {
        return '?';  // Unknown state
    }
}

// Print the timestamp with a message
void print_event(const char* event, pid_t pid, const char* name) {
    time_t now;
    time(&now);
    char *date = ctime(&now);
    date[strlen(date) - 1] = '\0';  // Remove newline character
    printf("[%s] PID %d (%s): %s\n", date, pid, name, event);
}

// Function to monitor processes
void monitor_processes() {
    DIR *dir;
    struct dirent *entry;
    char process_name[256];
    char prev_state[32768] = {0};  // Track the previous state of each pid

    while (1) {
        dir = opendir(PROC_DIR);
        if (dir == NULL) {
            perror("Cannot open /proc");
            return;
        }

        // Iterate through each process in /proc
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_DIR) {
                pid_t pid = atoi(entry->d_name);
                if (pid > 0) {
                    char state = get_process_state(pid);
                    get_process_name(pid, process_name);

                    // Process launch detection
                    if (prev_state[pid] == 0) {
                        print_event("Launched", pid, process_name);
                    }

                    // Waiting queue (sleeping) detection
                    if (state == 'S' && prev_state[pid] != 'S') {
                        print_event("Waiting queue (sleeping)", pid, process_name);
                    }

                    // Process finish detection (process no longer exists)
                    prev_state[pid] = state;
                }
            }
        }
        closedir(dir);

        // Sleep for a bit to avoid overloading the system
        usleep(500000);  // Sleep for 0.5 seconds
    }
}

int main() {
    monitor_processes();
    return 0;
}


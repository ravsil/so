#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#define PROC_DIR "/proc"
#define MAX_PID 32768
#define NAME_LEN 256

// Cache for process names
char process_name_cache[MAX_PID][NAME_LEN] = {{0}};

// Function to get the process name from /proc/[pid]/comm
void get_process_name(pid_t pid, char *name) {
    char path[40];
    snprintf(path, sizeof(path), "/proc/%d/comm", pid);
    
    FILE *f = fopen(path, "r");
    if (f) {
        fgets(name, NAME_LEN, f);
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
    struct timeval tv;
    gettimeofday(&tv, NULL);

    // Convert seconds to a human-readable format
    struct tm* now_tm = localtime(&tv.tv_sec);
    
    // Print the time with milliseconds
    printf("[%02d:%02d:%02d.%03ld] PID %d (%s): %s\n",
        now_tm->tm_hour, now_tm->tm_min, now_tm->tm_sec,
        tv.tv_usec / 1000, pid, name, event);
}


// Function to monitor processes
void monitor_processes() {
    DIR *dir;
    struct dirent *entry;
    char process_name[NAME_LEN];
    char prev_state[MAX_PID] = {0};  // Track the previous state of each pid
    int active_pids[MAX_PID] = {0};  // Track which pids are currently active

    while (1) {
        dir = opendir(PROC_DIR);
        if (dir == NULL) {
            perror("Cannot open /proc");
            return;
        }

        int current_pids[MAX_PID] = {0};  // Track current pids in this iteration

        // Iterate through each process in /proc
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_DIR) {
                pid_t pid = atoi(entry->d_name);
                if (pid > 0) {
                    char state = get_process_state(pid);
                    get_process_name(pid, process_name);

                    current_pids[pid] = 1;  // Mark this PID as active in this iteration

                    // Cache the process name for terminated detection
                    strncpy(process_name_cache[pid], process_name, NAME_LEN);

                    // Process launch detection
                    if (active_pids[pid] == 0) {
                        print_event("Launched", pid, process_name);
                    }

                    // Waiting queue (sleeping) detection
                    if (state == 'S' && prev_state[pid] != 'S') {
                        print_event("Waiting queue (sleeping)", pid, process_name);
                    }

                    prev_state[pid] = state;
                }
            }
        }

        // Detect terminated processes (those present in previous iteration but not in current)
        for (int pid = 1; pid < MAX_PID; pid++) {
            if (active_pids[pid] && !current_pids[pid]) {
                // Use cached name for terminated process
                print_event("Terminated", pid, process_name_cache[pid]);
                process_name_cache[pid][0] = '\0';  // Clear the cached name
            }
        }

        // Update active pids for the next iteration
        memcpy(active_pids, current_pids, sizeof(current_pids));

        closedir(dir);

        // Sleep for a bit to avoid overloading the system
        usleep(500000);  // Sleep for 0.5 seconds
    }
}

int main() {
    monitor_processes();
    return 0;
}

